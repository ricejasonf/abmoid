#ifndef ABMOID_AGENT_HPP
#define ABMOID_AGENT_HPP

#include <cstdint>
#include <functional>
#include <iterator>
#include <random>

namespace abmoid {

template <typename Agent>
class population_t;

template <typename TagType, typename IdType = uint_fast32_t>
class agent_t {
protected:
  friend class population_t<agent_t<TagType, IdType>>;
  IdType id;

  explicit agent_t(IdType id)
    : id(id)
  { }

public:
  using tag_type = TagType;
  using id_type = IdType;

  agent_t() = default;

  bool is_valid() const {
    return id != 0;
  }

  id_type get_id() const {
    return id;
  }

  bool operator==(agent_t const&) const = default;
};

class agent : public agent_t<void>
{
  friend class population_t<agent>;
  agent(id_type id)
    : agent_t<void>(id)
  { }
};

template <typename X>
concept Agent = requires(X x) {
  ([]<typename IdType, typename TagType>
    (agent_t<TagType, IdType> const&) { })(x);
};

template <typename Agent>
class population_t {
  using id_type = Agent::id_type;

  id_type N;

public:
  explicit population_t(id_type N)
    : N(N)
  { }

  class iterator {
    id_type current;
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = Agent;

    iterator() = default;
    explicit iterator(id_type c)
      : current(c)
    { }

    auto operator<=>(iterator const&) const = default;

    value_type operator*() const {
      return Agent(current);
    }

    iterator& operator++() {
      ++current;
      return *this;
    }

    iterator operator++(int) {
      iterator temp = *this;
      ++*this;
      return temp;
    }

    iterator& operator--() {
      --current;
      return *this;
    }

    iterator operator--(int) {
      iterator temp = *this;
      --*this;
      return temp;
    }

    iterator operator+(difference_type n) const {
      return iterator{current + n};
    }

    iterator operator-(difference_type n) const {
      return iterator{current - n};
    }

    friend
    iterator operator+(difference_type n, iterator const& itr) {
      return iterator{n + itr.current};
    }

    friend
    iterator operator-(difference_type n, iterator const& itr) {
      return iterator{n - itr.current};
    }

    iterator& operator+=(difference_type n) {
      current += n;
      return *this;
    }

    iterator& operator-=(difference_type n) {
      current -= n;
      return *this;
    }

    Agent operator[](difference_type n) const {
      return *iterator{current + n};
    }

    difference_type operator-(iterator const& other) const {
      return current - other.current;
    }
  };

  static_assert(std::random_access_iterator<iterator>);

  id_type size() const {
    return N;
  }

  iterator begin() const {
    return iterator{1};
  }

  iterator end() const {
    return iterator{N + 1};
  }

  template <typename Gen>
  agent select_random(Gen& gen) const {
    return Agent(std::uniform_int_distribution<id_type>(1, N)(gen));
  }
};

using population = population_t<agent>;

static_assert(std::ranges::random_access_range<population>);

}

template <abmoid::Agent Agent>
struct std::hash<Agent> {
  using IdType = Agent::id_type;
  using TagType = Agent::tag_type;
  uint32_t operator()(abmoid::agent_t<TagType, IdType> agent) const noexcept {
    return std::hash<IdType>{}(agent.get_id());
  }
};

#endif
