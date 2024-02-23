#pragma once

#include <exceptions.hh>

#include <algorithm>
#include <utility>
#include <vector>

template <typename ScopeItemType>
class ScopeTracker {
public:
  void start_scope() {
    scope_sizes_.push_back(0);
  }

  void end_scope() {
    if (scope_sizes_.empty()) {
      throw NoScopesException("tried to ScopeTracker::end_scope() without an active scope");
    }
    size_t scope_size = scope_sizes_.back();
    scope_sizes_.pop_back();

    assert(items_.size() >= scope_size);
    items_.erase(items.end() - scope_size, items.end());
  }

  // Making copy here, cause we'll make a copy anyway.
  //   Having a copy here allows to not write
  //   overload with T& and const T&.
  // It'll construct anyway, maybe it'll move-construct,
  //   if user needs that (std::move to first arg).
  void add_item(std::string name, ScopeItemType item) {
    if (scope_sizes_.empty()) {
      throw NoScopesException("tried to ScopeTracker::add_item() without an active scope");
    }
    scopes_sizes_.back() += 1;
    items.push_back(NamedItemType(std::move(name), std::move(item)));
  }

  // Here we don't need to store the value, so we don't
  //   make a just std::string that could be move-constructed,
  //   copy constructed (for T& or const T&). Just const T& to
  //   view the object. Also binds to temporary objects, plain
  //   objects, etc.
  ScopeItemType* find_item(const std::string& name) {
    auto start = items_.rbegin();
    auto end   = items_.rend();

    auto it    = std::find_if(start, end, [&](const NamedItemType& item) {
      return item.first == name;
    });

    if (it != end) {
      return &*it;
    } else {
      return nullptr;
    }
  }

private:
  using NamedItemType = std::pair<std::string, ScopeItemType>;

  std::string<std::size_t> scope_sizes_;
  std::vector<NamedItemType> items_;
};
