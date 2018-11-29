//
// Created by Alon on 19/08/2018.
//

#ifndef WET_V2_AVL_H
#define WET_V2_AVL_H

#include <iostream>
#include <assert.h>
#include <cmath>

#define NULL_HIEGHT (-1)
#define LEAF_HIEGHT (0)
#define BRANCH_HIEGHT (1)

// T must have both following compare functions >,==
// OR a comapre function(!)
template <class T>
class AVL {
public:
    struct Node {
        T *data;
        Node *parent, *left, *right;
        int h;
    };
    typedef int FCN(T const& a, T const& b);

    explicit AVL(FCN * const cmp_fcn) : cmp_fcn(cmp_fcn), root(new Node), number(0) {
        makeCleanNode(root);
    }
    ~AVL() {
        AVLDestructor(root, false);
    }
    // destroies the tree, if delete_data is true, deletes the data.
    void specialDestructor(bool delete_data = false) {
        AVLDestructor(root, delete_data);
    }

    Node* find(T *find) const {
        if (!root->data)
            return NULL;
        return findNode(root, find);
    }
    T* getData(T* data) const {
        Node *node = find(data);
        return node ? node->data : NULL;
    }
    void insert(T *data) {
        if (!root->data) { //only at first node!
            (root->data) = data;
            ++number;
            return;
        }
        insertNew(root, data);
    }
    void remove(T *data) {
        if (!data || !root->data)
            return;
        removeReplacement(findNode(root, data));
        if (!root) { // if the tree is empty: create a new root
            root = new Node;
            makeCleanNode(root);
        }
    }
    T* getMax() const {
        if (!root->data) return NULL;
        Node *node = root;
        while (node->right) {
            node = node->right;
        }
        return node->data;
    }
    int getAmount() const { return number; }
	// Create a tree from an ordered array at O(n) time and place.
    void newTree(T** data, int size) {
		specialDestructor(false);
		root = new Node();
        makeCleanNode(root, NULL, 0);
        makeNewSpecialTree(data, root, size);
    }
    T** getArrayInOrder() const {
        T** array(new T*[number + 1]);
        AVLArray(array, root);

        return array;
    };



private:
    FCN * const cmp_fcn;
    Node* root;
    int number;

    void makeCleanNode(Node *node, Node* parent = NULL, int h = LEAF_HIEGHT) {
        node->data = NULL;
        node->parent = parent;
        node->right = NULL;
        node->left = NULL;
        node->h = h;
    }
    AVL(AVL &);
    AVL &operator=(AVL&);
    void AVLDestructor(Node *node, bool delete_data = false) {
        if (!node) return;
        AVLDestructor(node->left, delete_data);
        AVLDestructor(node->right, delete_data);
        if (delete_data) {
            delete node->data;
            return;
        }
        delete node;
    }

    Node* findNode(Node *node, T *data) const {
        if (!node)
            return NULL;
        if (cmp_fcn(*(node->data), *data) == 0)
            return node; // found
        if (cmp_fcn(*(node->data), *data) > 0)
            return findNode(node->left, data);
        return findNode(node->right, data);
    }

    void insertNew(Node* node, T *data) {
        if (cmp_fcn(*(node->data), *data) == 0)
            return;
        else if (cmp_fcn(*(node->data), *data) > 0) {
            if (node->left)
                insertNew(node->left, data);
            else {
                node->left = new Node();
                makeCleanNode(node->left, node);
                ((node->left)->data) = data;
                ++number;
                fixTree(node);
            }
        } else {
            if (node->right)
                insertNew(node->right, data);
            else {
                node->right = new Node();
                makeCleanNode(node->right, node);
                ((node->right)->data) = data;
                ++number;
                fixTree(node);
            }
        }
    }

    void removeReplacement(Node* node) {
        if (!node)
            return;
        --number;
        Node* fix = node->parent;

        if (!(node->left) && !(node->right)) {
            // case a leaf
            removeNodeFromParent(fix, node);
            if (root == node) {
                root->data = NULL;
                return;
            }
            delete node;
        } else if (!(node->left)) {
            // case 1 branch
            moveNode(node->right, node);
        } else if (!(node->right)) {
            // case 1 branch
            moveNode(node->left, node);
        } else {
            // case 2 branches
            removeReplaceWithMin(node);
        }

        fixTree(fix);
    }
    void removeReplaceWithMin(Node *node) {
        Node *replace = node->right;
        while (replace->left) {
            replace = replace->left;
        }
        node->data = replace->data;
        if (replace->right) {
            if (node->right == replace) {
                node->right = replace->right;
            } else {
                (replace->parent)->left = replace->right;
            }
            (replace->right)->parent = replace->parent;
            fixTree(replace->right); //todo needed here?
        } else {
            removeNodeFromParent(replace->parent, replace);
        }
        delete replace;
    }

    void fixTree(Node* node) {
        fixHeights(node);
        while (node) {
            int old_h = node->h;
            int BF = calcBF(node);
            bool rotated = false;
            if (BF == 2) {
                node = checkRotateL(node);
                rotated = true;
            } else if (BF == -2) {
                node = checkRotateR(node);
                rotated = true;
            }
            if (rotated) {
                if (!(node->parent)) {
                    // case *root* has been rotated and needs to be redefined
                    root = node;
                }
                fixHeight(node);
                if (node->h == old_h)
                    break;
            }

            node = node->parent;
        }
    }
    void fixHeights(Node* node) {
        if (node) {
            fixHeight(node);
            assert(node != node->parent);
            fixHeights(node->parent);
        }
    }
    Node* checkRotateL(Node* node) {
        if (calcBF(node->left) < 0)
            // Rotate LR
            node->left = rotateLeft(node->left);
        //Rotate LL
        return rotateRight(node);
    }
    Node* checkRotateR(Node* node) {
        if (calcBF(node->right) > 0)
            // Rotate RL
            node->right = rotateRight(node->right);
        //Rotate RR
        return rotateLeft(node);
    }
    Node* rotateLeft(Node *node) {
        //Rotate to the LEFT one time.
        Node *head = node->right;
        //swap parent
        head->parent = node->parent;
        if (head->parent) {
            if ((head->parent)->left == node)
                (head->parent)->left = head;
            else
                (head->parent)->right = head;
        }
        //swap A(left)<=>B(right)
        node->right = head->left;
        if (node->right)
            (node->right)->parent = node;
        //swap A<=>B
        node->parent = head;
        head->left = node;

        fixHeight(node);
        return head;
    }
    Node* rotateRight(Node *node) {
        //Rotate to the RIGHT one time.
        Node *head = node->left;
        //swap parent
        head->parent = node->parent;
        if (head->parent) {
            if ((head->parent)->left == node)
                (head->parent)->left = head;
            else
                (head->parent)->right = head;
        }
        //swap A(right)<=>B(left)
        node->left = head->right;
        if (node->left)
            (node->left)->parent = node;
        //swap A<=>B
        node->parent = head;
        head->right = node;

        fixHeight(node);
        return head;
    }

// Removes the pointer to the node from the parent's node(if exist)
    void removeNodeFromParent(Node* parent, Node* node) {
        if (!parent || !node) return;
        if (parent->right == node)
            parent->right = NULL;
        else if (parent->left == node)
            parent->left = NULL;
    }
    void fixHeight(Node* node) {
        if (!node)
            return;
        node->h = LEAF_HIEGHT;
        if (node->left)
            node->h = BRANCH_HIEGHT + (node->left)->h;
        if (node->right && (node->right)->h > (node->h - BRANCH_HIEGHT))
            node->h = BRANCH_HIEGHT + (node->right)->h;
    }
    int calcBF(Node* node) const {
        if (!node)
            return NULL_HIEGHT;
        int BF = 0;
        if (node->left)
            BF += (node->left)->h;
        else
            BF += NULL_HIEGHT;
        if (node->right)
            BF -= (node->right)->h;
        else
            BF -= NULL_HIEGHT;
        return BF;
    }
    void moveNode(Node* from, Node* to) {
        if (!from || !to)
            return;
        //to->parent = from->parent;
        to->right = from->right;
        if (from->right) {
            (from->right)->parent = to;
        }
        to->left = from->left;
        if (from->left) {
            (from->left)->parent = to;
        }
        to->data = from->data;
        to->h = from->h;
        delete from;
    }

    T** AVLArray(T** array, Node* node) const {
        if (!node) return array;
        array = AVLArray(array, node->left);
        *array = (node->data);
        ++array;
        array = AVLArray(array, node->right);
        return array;
    }

    void buildEmptyFullTree(Node *node, int height) {
        if (height < 1 || !node) {
            return;
        }
        node->left = new Node();
        makeCleanNode(node->left, node, height-1);
        node->right = new Node();
        makeCleanNode(node->right, node, height-1);

        buildEmptyFullTree(node->left, height-1);
        buildEmptyFullTree(node->right, height-1);
        fixHeight(node);
    }
    int deleteExtraLeaves(Node *node, int number) {
        if (!node || number < 1) {
            return number;
        }
        int old_number = number;
        if (node->h == LEAF_HIEGHT) {
            removeNodeFromParent(node->parent, node);
            delete node;
            --number;
            return number;
        } else {
            number = deleteExtraLeaves(node->right, number);
            number = deleteExtraLeaves(node->left, number);
        }
        if (number != old_number) {
            fixHeight(node);
        }
        return number;
    }
    T** putDataInOrder(Node* node, T** data, int &size) {
        if (!node || !data) {
            return data;
        }
        data = putDataInOrder(node->left, data, size);
        node->data = *data;
        data++;
        --size;
        data = putDataInOrder(node->right, data, size);
        return data;
    }
    void makeNewSpecialTree(T** data, Node* &root, int size) {
        int height = floor(log2(size));
        buildEmptyFullTree(root, height);
        deleteExtraLeaves(root, pow(2,height+1)-size-1);
        number = size;
        putDataInOrder(root, data, size);
        assert(size == 0);
    }

};


#endif //WET_V2_AVL_H
