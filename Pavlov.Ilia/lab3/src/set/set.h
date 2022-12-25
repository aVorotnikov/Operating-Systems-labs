#pragma once

template <class T>
class Set {
public:
  virtual bool Add(const T& item) = 0;
  virtual bool Remove(const T& item) = 0;
  virtual bool Contains(const T& item) const = 0;
  virtual bool Empty() const = 0;
  virtual ~Set() {};
};
