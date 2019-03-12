#include <sched/pgrp.h>
#include <sched/process.h>
#include <sched/spinlock.h>
#include <mm/heap.h>
#include <mm/memset.h>
#include <uapi/errno.h>
#include <uapi/syscalls.h>

#include <print.h>

static struct RbNode *pgRoot;

static struct Session *firstSession;
static struct Session *lastSession;
static spinlock_t sessionLock; //also used for groups

static void unlinkGroup(struct PGroup *grp) {
	if (grp->prev) {
		grp->prev->next = grp->next;
	} else {
		grp->s->first = grp->next;
	}
	if (grp->next) {
		grp->next->prev = grp->prev;
	} else {
		grp->s->last = grp->prev;
	}
	if (!grp->s->first && !grp->s->last) {
		//delete session
		
		if (grp->s->prev) {
			grp->s->prev->next = grp->s->next;
		} else {
			firstSession = grp->s->next;
		}
		if (grp->s->next) {
			grp->s->next->prev = grp->s->prev;
		} else {
			lastSession = grp->s->prev;
		}
		kfree(grp->s);
	}
}

void leaveGroup(struct Process *proc) { //also called on process cleanup
	struct PGroup *grp = proc->group;
	if (!grp) {
		return; //dont leave group on fork
	}

	if (proc->grpPrev) {
		proc->grpPrev->grpNext = proc->grpNext;
	} else {
		grp->first = proc->grpNext;
	}
	if (proc->grpNext) {
		proc->grpNext->grpPrev = proc->grpPrev;
	} else {
		grp->last = proc->grpPrev;
	}
	proc->grpNext = NULL;
	proc->grpPrev = NULL;

	if (!grp->first && !grp->last) {
		//delete group
		unlinkGroup(grp);
		rbDelete(&pgRoot, grp->pgid);
		kfree(grp);
	}
	proc->group = NULL;
}

int setpgid(struct Process *proc, pid_t pgid) {
	//temp
	if (pgid < 0) {
		return -EINVAL;
	}

	int error = 0;
	if (!pgid) {
		pgid = proc->pid;
	}

	acquireSpinlock(&proc->lock);

	struct PGroup *newGroup;
	struct Session *s;
	acquireSpinlock(&sessionLock);

	if (proc->group && proc->group->first == proc && proc->group->last == proc) {
		rbDelete(&pgRoot, proc->pid);
		proc->group->pgid = pgid;
		newGroup->node.value = pgid;
		rbInsert(&pgRoot, (struct RbNode *)proc);
		goto releaseSessionLock;
	}
	if (proc->pid != 1) {
		s =  proc->group->s;
	} else {
		//create new session, should only happen on execInit()
		s = kzalloc(sizeof(*s));
		s->sid = pgid;
		firstSession = s;
		lastSession = s;
	}

	if (proc->pid != pgid) {
		newGroup = (struct PGroup *)rbSearch(pgRoot, pgid);
		if (newGroup) {
			//Add to group
			if (newGroup->last) {
				newGroup->last->grpNext = proc;
				proc->grpPrev = newGroup->last;
			} else {
				proc->grpPrev = NULL;
				newGroup->first = proc;
			}
			newGroup->last = proc;
			proc->grpNext = NULL;

			proc->pgid = pgid;
			proc->sid = newGroup->s->sid;
			proc->group = newGroup;
			goto releaseSessionLock;
		}
	}

	newGroup = kzalloc(sizeof(*newGroup));
	if (!newGroup) {
		error = -ENOMEM;
		goto releaseSessionLock;
	}
	
	leaveGroup(proc);
	newGroup->first = proc;
	newGroup->last = proc;
	newGroup->pgid = pgid;
	newGroup->node.value = pgid;
	newGroup->s = s;
	proc->pgid = newGroup->pgid;
	proc->group = newGroup;
	proc->grpNext = NULL;
	proc->grpPrev = NULL;

	rbInsert(&pgRoot, (struct RbNode *)newGroup);

	//Add group to session
	if (s->last) {
		s->last->next = newGroup;
		newGroup->prev = s->last;
	} else {
		s->first = newGroup;
	}
	s->last = newGroup;

	releaseSessionLock:
	releaseSpinlock(&sessionLock);
	//release:
	releaseSpinlock(&proc->lock);
	return error;
}

struct PGroup *getPGroup(pid_t pgid) {
	return (struct PGroup *)rbSearch(pgRoot, pgid);
}

int sysSetpgid(pid_t pid, pid_t pgid) {
	struct Process *curProc = getCurrentThread()->process;
	struct Process *proc;
	if (pid && pid != curProc->pid) {
		proc = getProcFromPid(pid);
		if (!proc || proc->ppid != curProc->pid) {
			return -ESRCH;
		}
	} else {
		proc = curProc;
	}

	return setpgid(proc, pgid);
}

pid_t sysSetsid(void) {
	pid_t ret = 0;
	struct Process *proc = getCurrentThread()->process;
	acquireSpinlock(&proc->lock);

	if (proc->sid == proc->pid || proc->pgid == proc->pid) {
		ret = -EPERM;
		goto release;
	}

	//do allocations first
	struct Session *newSession = kzalloc(sizeof(*newSession));
	if (!newSession) {
		ret = -ENOMEM;
		goto release;
	}
	struct PGroup *newGroup = kzalloc(sizeof(*newGroup));
	if (!newGroup) {
		ret = -ENOMEM;
		goto deallocSession;
	}

	acquireSpinlock(&sessionLock);
	leaveGroup(proc);

	newSession->sid = proc->pid;
	newSession->first = newGroup;
	newSession->last = newGroup;
	newGroup->pgid = proc->pid;
	newGroup->node.value = proc->pid;
	newGroup->s = newSession;
	newGroup->first = proc;
	newGroup->last = proc;
	
	if (lastSession) {
		lastSession->next = newSession;
		newSession->prev = lastSession;
	} else {
		firstSession = newSession;
	}
	lastSession = newSession;
	releaseSpinlock(&sessionLock);

	proc->sid = proc->pid;
	proc->pgid = proc->pid;
	proc->group = newGroup;
	proc->grpNext = NULL;
	proc->grpPrev = NULL;

	ret = proc->sid;
	goto release;

	deallocSession:
	kfree(newSession);
	release:
	releaseSpinlock(&proc->lock);
	return ret;
}

/*
Get pid, ppid, pgid or sid from process pointed to by pid
if pid == 0 then get from current process
*/
pid_t sysGetId(pid_t pid, int which) {
	pid_t ret = 0;
	thread_t curThread = getCurrentThread();
	struct Process *proc = curThread->process;
	if (pid) {
		proc = getProcFromPid(pid);
		if (!proc) {
			return -ESRCH;
		}
	}
	switch (which) {
		case SYSGETID_PID:
			ret = proc->pid;
			break;
		case SYSGETID_PPID:
			ret = proc->ppid;
			break;
		case SYSGETID_PGID:
			ret = proc->pgid;
			break;
		case SYSGETID_SID:
			ret = proc->sid;
			break;
		case SYSGETID_UID:
			ret = proc->cred.ruid;
			break;
		case SYSGETID_EUID:
			ret = proc->cred.euid;
			break;
		case SYSGETID_GID:
			ret = proc->cred.rgid;
			break;
		case SYSGETID_EGID:
			ret = proc->cred.egid;
			break;
		default:
			ret = -EINVAL;
			break;
	}
	return ret;
}

int sysSetId(pid_t id, int which) {
	if (id < 0) {
		return -EINVAL;
	}
	thread_t curThread = getCurrentThread();
	struct ProcessCred *cred = &curThread->process->cred;
	switch (which) {
		case SYSSETID_UID:
			if (cred->euid) {
				if (id == cred->ruid || id == cred->suid) {
					cred->euid = id;
					return 0;
				}
				return -EPERM;
			}
			cred->ruid = id;
			cred->euid = id;
			cred->suid = id;
			return 0;
		case SYSSETID_EUID:
			if (cred->euid && id != cred->ruid && id != cred->suid) {
				return -EPERM;
			}
			cred->euid = id;
			return 0;
		case SYSSETID_GID:
			if (cred->egid) {
				if (id == cred->rgid || id == cred->sgid) {
					cred->egid = id;
					return 0;
				}
				return -EPERM;
			}
			cred->rgid = id;
			cred->egid = id;
			cred->sgid = id;
			return 0;
		case SYSSETID_EGID:
			if (cred->egid && id != cred->rgid && id != cred->sgid) {
				return -EPERM;
			}
			cred->egid = id;
			return 0;
		default:
			return -EINVAL;
	}
}

int sysSetReuid(uid_t ruid, uid_t euid, int which) {
	thread_t curThread = getCurrentThread();
	struct ProcessCred *cred = &curThread->process->cred;
	if (which) {
		gid_t rgid = ruid;
		gid_t egid = euid;
		if (cred->egid) {
			bool setEgid = false;
			if (egid >= 0 && (egid == cred->rgid || egid == cred->egid || egid == cred->sgid)) {
				setEgid = true;
			} else if (egid >= 0) {
				return -EPERM;
			}
			if (rgid >= 0 && (rgid == cred->rgid || rgid == cred->egid)) {
				cred->rgid = rgid;
			} else if (egid >= 0) {
				return -EPERM;
			}
			if (setEgid) cred->egid = egid;
		} else {
			if (rgid >= 0) {
				cred->rgid = rgid;
			}
			if (egid >= 0) {
				cred->egid = egid;
			}
		}
	} else {
		if (cred->euid) {
			bool setEuid = false;
			if (euid >= 0 && (euid == cred->ruid || euid == cred->euid || euid == cred->suid)) {
				//cred->euid = euid;
				setEuid = true;
			} else if (euid >= 0) {
				return -EPERM;
			}
			if (ruid >= 0 && (ruid == cred->ruid || ruid == cred->euid)) {
				cred->ruid = ruid;
			} else if (euid >= 0) {
				return -EPERM;
			}
			if (setEuid) cred->euid = euid;
		} else {
			if (ruid >= 0) {
				cred->ruid = ruid;
			}
			if (euid >= 0) {
				cred->euid = euid;
			}
		}
	}
	return 0;
}