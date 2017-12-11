/*
 *  MCS Queue Lock
 */

#include <iostream>
#include <thread>
#include <atomic>

using namespace std;

class Node {
    public:
        Node () : locked(false), next(NULL) {}
        void setLocked(bool value) {
            locked.store(value, memory_order_release);
        }
        bool isLocked() {
            return locked.load(memory_order_acquire);
        }
        Node* getNext() {
            return next;
        }
        void setNext(Node *nextNode) {
            next = nextNode;
        }
    private:
        atomic<bool> locked;
        Node *next;
};

class QueueLock {
    public:
        QueueLock() : tail(NULL) {}
        void lock(Node *thrdNode) {
            Node *qnode = thrdNode;
            Node *pred = tail.exchange(qnode, memory_order_acq_rel);
            if (pred != NULL) {
                qnode->setLocked(true);
                pred->setNext(qnode);
                while (qnode->isLocked()) {}
            }
        }
        void unlock(Node *thrdNode) {
            Node *qnode = thrdNode;
            if (qnode->getNext() == NULL) {
                if (tail.compare_exchange_weak(qnode, NULL, memory_order_acq_rel)) {
                    return;
                }
                while (qnode->getNext() == NULL) {}
            }
            qnode->getNext()->setLocked(false);
            qnode->setNext(NULL);
        }
    private:
        atomic<Node*> tail;
};
