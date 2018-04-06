#include <sched/pgrp.h>
#include <sched/spinlock.h>
#include <mm/heap.h>
#include <mm/memset.h>
#include <uapi/errno.h>

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
	if (proc->group) {
		s =  proc->group->s;
	} else {
		//create new session, should only happen on execInit()
		s = kzalloc(sizeof(*s));
		s->sid = pgid;
		firstSession = s;
		lastSession = s;
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
	release:
	releaseSpinlock(&proc->lock);
	return error;
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

	ret = proc->sid;
	goto release;

	deallocSession:
	kfree(newSession);
	release:
	releaseSpinlock(&proc->lock);
	return ret;
}

