#pragma once

#include "set.h"

#include <pthread.h>
#include <functional>

template <class T, class Compare = std::less<T>>
class SetCoarseSync : public Set<T> {
private:
  struct Node {
    struct Node* next = nullptr;
    T data;
    Node(const T& data) : data(data) {}
  };
  
  Node* head = nullptr;
  mutable pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  Compare compare;

  bool is_equal(const T& lhs, const T& rhs) const {
    return !compare(lhs, rhs) && !compare(rhs, lhs);
  }

public:
  SetCoarseSync(const Compare& compare = Compare()): compare(compare) {}

  virtual bool Add(const T& item) override {
    pthread_mutex_lock(&mutex);

    if (!head) {
      head = new Node(item);
      pthread_mutex_unlock(&mutex);
      return true;
    }

    if (!compare(head->data, item) && !is_equal(head->data, item)) {
      Node* tmp = head;
      head = new Node(item);
      head->next = tmp;
      pthread_mutex_unlock(&mutex);
      return true;
    }
    Node* node = head;
    while (node->next && compare(node->next->data, item))
      node = node->next;
    
    if (is_equal(node->data, item)) {
      pthread_mutex_unlock(&mutex);
      return false;
    }

    Node* tmp = node->next;
    node->next = new Node(item);
    node->next->next = tmp;

    pthread_mutex_unlock(&mutex);

    return true;
  }

  virtual bool Remove(const T& item) override {
    pthread_mutex_lock(&mutex);

    if (!head) {
      pthread_mutex_unlock(&mutex);
      return false;
    }

    if (is_equal(head->data, item)) {
      Node* tmp = head->next;
      delete head;
      head = tmp;
      pthread_mutex_unlock(&mutex);
      return true;
    }

    Node* prev = head;
    Node* node = head->next;

    while (node && compare(node->data, item)) {
      prev = node;
      node = node->next;
    }

    if (node && is_equal(node->data, item)) {
      prev->next = node->next;
      delete node;
      pthread_mutex_unlock(&mutex);
      return true;
    }

    pthread_mutex_unlock(&mutex);

    return false;
  }

  virtual bool Contains(const T& item) const override {
    pthread_mutex_lock(&mutex);

    const Node* node = head;
    while (node && compare(node->data, item))
      node = node->next;

    bool res = node && is_equal(node->data, item);
    
    pthread_mutex_unlock(&mutex);

    return res;
  }

  virtual bool Empty() const override {
    return head == nullptr;
  }

  ~SetCoarseSync() {
    Node* node = head;
    Node* next;
    while (node) {
      next = node->next;
      delete node;
      node = next;
    }
    pthread_mutex_destroy(&mutex);
  }
};