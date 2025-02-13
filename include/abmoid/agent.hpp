#ifndef ABMOID_AGENT_HPP
#define ABMOID_AGENT_HPP

namespace abmoid {

template <typename Agent>
class population;

template <typename T>
class agent_t {
  friend class population<agent_t<T>>;
  using id_type = uint_fast32_t;
  id_type id;

  explicit agent_t(id_type id)
    : id(id)
  { }

public:

  agent_t() = default;

  bool is_valid() const {
    return id != 0;
  }

  id_type get_id() const {
    return id;
  }

  bool operator==(agent const&) const = default;
};

struct agent : public agent_t<void>
{ };

template <typename Agent>
class population {
  using id_type = Agent::id_type;

  id_type size;

public:
  explicit population(id_type size)
    : size(size)
  { }

  class iterator {
    id_type current;
  public:
    using difference_type = id_type;
    using value_type = Agent;

    value_type operator*() const {
      return Agent{current};
    }

    iterator& operator++() {
      ++current;
      return *this;
    }

    void operator++(int) {
      ++*this;
    }
  };

  iterator begin() {
    return iterator{1};
  }

  iterator end() {
    return iterator{N + 1};
  }
};

}

template <>
struct std::hash<abmoid::agent> {
  uint32_t operator()(abmoid::agent agent) const noexcept {
    return std::hash<abmoid::agent::id_type>{}(agent.get_id());
  }
};

#endif
