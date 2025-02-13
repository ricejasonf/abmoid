#ifndef ABMOID_AGENT_COMPONENT_HPP
#define ABMOID_AGENT_COMPONENT_HPP

#include <cassert>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace abmoid {

template <typename Value, typename Agent = agent>
class agent_component {
  using value_storage = std::vector<Value>;
  using agent_storage = std::vector<Agent>;
  using lookup_storage = std::unordered_map<Agent, unsigned>;
  using index_t = unsigned;

  value_storage values;
  agent_storage agents;
  lookup_storage lookup;

public:
  using iterator = value_storage::iterator;

  void clear() {
    values.clear();
    agents.clear();
    lookup.clear();
  }

  auto size() const { return values.size(); }
  iterator begin() const { return values.begin(); }
  iterator begin() { return values.begin(); }
  iterator end() const { return values.end(); }
  iterator end() { return values.end(); }
  iterator erase(iterator itr) {
    index_t index = std::distance(values.begin(), itr);
    auto agent_itr = agents.begin() + index;
    auto lookup_itr = lookup.find(get_agent(itr));
    assert(agents.size() > index &&
        "corresponding agent entry should exist");
    assert(lookup_itr != lookup.end() &&
        "component must exist to erase it");

    if (size() == 1) {
      clear();
      return end();
    }

    lookup.erase(lookup_itr);

    // Remove the value and agent entries by
    // swapping with the last element so we can efficiently
    // remove the element without reindexing everything.
    using std::swap;
    swap(*itr, values.back());
    swap(*agent_itr, agents.back());
    lookup[*agent_itr] = index;
    values.pop_back();
    agents.pop_back();

    // Since itr was swapped with the back, we do not increment.
    return itr;
  }

  bool contains(Agent a) {
    return lookup.contains(a);
  }

  template <typename V>
  Value& create(Agent a, V&& value) {
    assert(!contains(a) && "only one component per entity is allowed");
    lookup[a] = values.size();
    values.push_back(std::forward<V>(value));
    agents.push_back(a);
    return values.back();
  }

  Agent get_agent(index_t index) const {
    assert(index < agents.size());
    return agents[index];
  }

  Agent get_agent(iterator itr) {
    return get_agent(std::distance(values.begin(), itr));
  }
};

template <typename T>
concept Empty = std::is_empty_v<T> && std::is_default_constructible_v<T>;

template <Empty Value, typename Agent>
class agent_component<Value, Agent> {
  using agent_storage = std::vector<Agent>;
  using lookup_storage = std::unordered_map<Agent, unsigned>;
  using index_t = unsigned;

  value_storage values;
  agent_storage agents;
  lookup_storage lookup;

public:
  // Just iterate the agents.
  using iterator = agent_storage::const_iterator;

  void clear() {
    values.clear();
    agents.clear();
    lookup.clear();
  }

  auto size() const { return agents.size(); }
  iterator begin() const { return agents.begin(); }
  iterator begin() { return agents.begin(); }
  iterator end() const { return agents.end(); }
  iterator end() { return agents.end(); }
  iterator erase(iterator itr) {
    index_t index = std::distance(agents.begin(), itr);
    auto agent_itr = agents.begin() + index;
    auto lookup_itr = lookup.find(*itr);
    assert(agents.size() > index &&
        "corresponding agent entry should exist");
    assert(lookup_itr != lookup.end() &&
        "component must exist to erase it");

    if (size() == 1) {
      clear();
      return end();
    }

    lookup.erase(lookup_itr);

    // Remove the value and agent entries by
    // swapping with the last element so we can efficiently
    // remove the element without reindexing everything.
    using std::swap;
    swap(*itr, agents.back());
    lookup[*itr] = index;
    agents.pop_back();

    // Since itr was swapped with the back, we do not increment.
    return itr;
  }

  bool contains(Agent a) {
    return lookup.contains(a);
  }

  Value create(Agent a, Value)
    assert(!contains(a) && "only one component per entity is allowed");
    lookup[a] = agents.size();
    agents.push_back(a);
    return Value{};
  }

  Agent get_agent(index_t index) const {
    assert(index < agents.size());
    return agents[index];
  }

  Agent get_agent(iterator itr) {
    return get_agent(std::distance(values.begin(), itr));
  }
};

}

#endif
