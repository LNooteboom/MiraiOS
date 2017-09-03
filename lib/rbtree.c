/*
Red-black tree implementation for MiraiOS

Copyright (c) 2017 Luke Nooteboom

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdbool.h>
#include <stddef.h>
#include <lib/rbtree.h>

static void bstInsert(struct RbNode *root, struct RbNode *newNode) {
	while (true) {
		if (newNode->value < root->value) {
			if (root->leftChild) {
				root = root->leftChild;
			} else {
				root->leftChild = newNode;
				newNode->parent = root;
				break;
			}
		} else {
			if (root->rightChild) {
				root = root->rightChild;
			} else {
				root->rightChild = newNode;
				newNode->parent = root;
				break;
			}
		}
	}
}

static void rbLeftRotate(struct RbNode **root, struct RbNode *node) {
	struct RbNode *rChild = node->rightChild; //needs to exist
	struct RbNode *parent = node->parent;
	node->rightChild = rChild->leftChild;
	if (rChild->leftChild)
		rChild->leftChild->parent = node;
	rChild->leftChild = node;
	rChild->parent = node->parent;
	node->parent = rChild;
	if (parent) {
		if (node->value < parent->value)
			parent->leftChild = rChild;
		else
			parent->rightChild = rChild;
	} else {
		*root = rChild;
	}
}
static void rbRightRotate(struct RbNode **root, struct RbNode *node) {
	struct RbNode *lChild = node->leftChild; //needs to exist
	struct RbNode *parent = node->parent;
	node->leftChild = lChild->rightChild;
	if (lChild->rightChild)
		lChild->rightChild->parent = node;
	lChild->rightChild = node;
	lChild->parent = node->parent;
	node->parent = lChild;
	if (parent) {
		if (node->value < parent->value)
			parent->leftChild = lChild;
		else
			parent->rightChild = lChild;
	} else {
		*root = lChild;
	}
}

void rbInsert(struct RbNode **root, struct RbNode *newNode) {
	newNode->leftChild = NULL;
	newNode->rightChild = NULL;
	if (!(*root)) {
		*root = newNode;
		(*root)->parent = NULL;
		(*root)->isRed = false;
		return;
	}
	newNode->isRed = true;

	bstInsert(*root, newNode);

	struct RbNode *parent = newNode->parent;
	struct RbNode *grandParent;
	struct RbNode *uncle;
	while (true) {
		if (!parent || !parent->isRed) {
			if (!parent) {
				newNode->isRed = false;
				*root = newNode;
			}
			return;
		}
		grandParent = parent->parent; //parent is red so can't be root -> grandparent exists
		uncle = (parent->value < grandParent->value)? grandParent->rightChild : grandParent->leftChild;
		if (uncle && uncle->isRed) {
			//recolor
			parent->isRed = false;
			uncle->isRed = false;
			grandParent->isRed = true;
			//recur to grandparent
			newNode = grandParent;
			parent = newNode->parent;
			continue;
		}
		//struct Node *gg = grandParent->parent;
		if (parent->value < grandParent->value) {
			if (newNode->value >= parent->value) {
				//left rotate parent
				rbLeftRotate(root, parent);
			}
			//right rotate grandParent
			rbRightRotate(root, grandParent);
			
		} else {
			if (newNode->value < parent->value) {
				//right rotate parent
				rbRightRotate(root, parent);
			}
			//left rotate grandParent
			rbLeftRotate(root, grandParent);
		}
		bool gCol = grandParent->isRed;
		grandParent->isRed = grandParent->parent->isRed;
		grandParent->parent->isRed = gCol;
		newNode = grandParent->parent;
		parent = newNode->parent;
	}
}

struct RbNode *rbSearch(struct RbNode *root, unsigned long value) {
	struct RbNode *curNode = root;
	while (true) {
		if (!curNode)
			return NULL;
		if (value == curNode->value) {
			return curNode;
		} 
		if (value < curNode->value) {
			curNode = curNode->leftChild;
		} else {
			curNode = curNode->rightChild;
		}
	}
}

static struct RbNode *bstDelete(struct RbNode **root, /*out*/ struct RbNode **successor, struct RbNode **child, unsigned long value) {
	struct RbNode *del = rbSearch(*root, value);
	if (!del)
		return NULL; //not found

	if (del->leftChild && del->rightChild) {
		//find successor
		struct RbNode *s = del->rightChild;
		while (s->leftChild) {
			s = s->leftChild;
		}

		if (s->value < s->parent->value)
			s->parent->leftChild = NULL;
		else
			s->parent->rightChild = NULL;
		*child = s->rightChild;
		*successor = s;
		return del;
	} else if (del->leftChild || del->rightChild) {
		struct RbNode *child2;
		if (del->leftChild)
			child2 = del->leftChild;
		else
			child2 = del->rightChild;
		child2->parent = del->parent;

		if (del->parent) {
			if (child2->value < del->parent->value)
				del->parent->leftChild = child2;
			else
				del->parent->rightChild = child2;
		} else {
			*root = child2;
		}
		*child = child2;
		*successor = NULL;
		return del;
	}
	//del is leaf node
	if (del->parent) {
		if (del->value < del->parent->value)
			del->parent->leftChild = NULL;
		else
			del->parent->rightChild = NULL;
	} else {
		*root = NULL;
	}
	*child = NULL;
	*successor = NULL;
	return del;
}

struct RbNode *rbDelete(struct RbNode **root, unsigned long value) {
	struct RbNode *successor;
	struct RbNode *child;
	struct RbNode *del2 = bstDelete(root, &successor, &child, value);
	if (!del2)
		return NULL; //not found;
	struct RbNode *del;
	if (successor)
		del = successor;
	else
		del = del2;
	
	struct RbNode *parent = del->parent;
	if (del->isRed || (child && child->isRed)) {
		if (child) {
			child->isRed = false;
		}
		//return del2;
		goto out;
	}
	struct RbNode *sibling;
	bool childRight;
	while (true) {
		if (!parent) {
			*root = child;
			child->parent = NULL;
			break;
		}
		//child is double-black
		childRight = false;
		if (child) {
			if (child->value < parent->value) {
				sibling = parent->rightChild;
				childRight = true;
			} else {
				sibling = parent->leftChild;
			}
		} else {
			if (parent->leftChild) {
				sibling = parent->leftChild;
				childRight = true;
			} else {
				sibling = parent->rightChild;
			}
		}
		//sibling must exist
		if (sibling->isRed) {
			if (childRight) {
				//sibling left
				rbRightRotate(root, parent);
				parent->isRed = true;
				sibling->isRed = false;
				sibling = parent->leftChild;
			} else {
				rbLeftRotate(root, parent);
				parent->isRed = true;
				sibling->isRed = false;
				sibling = parent->rightChild;
			}
		}
		if (childRight) {
			//sibling left
			if (sibling->leftChild && sibling->leftChild->isRed) {
				//left-left
				rbRightRotate(root, parent);
				//sibling is now parent
				sibling->leftChild->isRed = false;
				break;
			} else if (sibling->rightChild && sibling->rightChild->isRed) {
				//right-left
				rbLeftRotate(root, sibling);
				sibling->parent->isRed = false; //parent used to be right child
				rbRightRotate(root, parent);
				break;
			}
		} else {
			if (sibling->rightChild && sibling->rightChild->isRed) {
				//right-right
				rbLeftRotate(root, parent);
				//sibling is now parent
				sibling->rightChild->isRed = false;
				break;
			} else if (sibling->leftChild && sibling->leftChild->isRed) {
				//right-left
				rbRightRotate(root, sibling);
				sibling->parent->isRed = false; //parent used to be left child
				rbLeftRotate(root, parent);
				break;
			}
		}
		//both of it's children are black
		sibling->isRed = true;
		if (parent->isRed) {
			parent->isRed = false;
			break;
		}
		child = parent;
		parent = child->parent;
	}
	out:
	if (successor) {
		successor->parent = del2->parent;
		successor->leftChild = del2->leftChild;
		successor->rightChild = del2->rightChild;
		successor->isRed = del2->isRed;
		if (del2->parent) {
			if (del2->value < del2->parent->value) {
				del2->parent->leftChild = successor;
			} else {
				del2->parent->rightChild = successor;
			}
		} else {
			*root = successor;
		}
	}
	return del2;
}