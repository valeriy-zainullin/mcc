#pragma once

#include <exception>
#include <string>

// Must write public, otherwise
//   "catch" keyword won't catch an exception
//   of our type as std::exception&.
//   https://stackoverflow.com/a/50133665

class DescribedException : public std::exception {
public:
  DescribedException() = delete;

  template <typename T> DescribedException(T msg) : msg_(std::move(msg)) {}

  virtual const char *what() const noexcept override { return msg_.c_str(); }

private:
  std::string msg_;
};

class NotImplementedException : public DescribedException {
public:
  using DescribedException::DescribedException;
};

class SemanticProblemException : public DescribedException {
public:
  using DescribedException::DescribedException;
};

class RuntimeProblemException : public DescribedException {
public:
  using DescribedException::DescribedException;
};

class NoScopesException : public DescribedException {
public:
  using DescribedException::DescribedException;
};
