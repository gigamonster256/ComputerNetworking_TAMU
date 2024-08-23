#ifndef _SBCP_HPP_
#define _SBCP_HPP_

#include <cstddef>
#include <cstdint>
#include <iterator>

#define SBCP_VERSION 3

#define SBCP_MAX_PAYLOAD_LENGTH 1024

#define SBCP_MAX_USERNAME_LENGTH 16
#define SBCP_MAX_MESSAGE_LENGTH 512
#define SBCP_MAX_REASON_LENGTH 32

namespace sbcp {

typedef class Message {
 public:
  typedef class Attribute {
   public:
    typedef enum class Type : uint16_t {
      USERNAME = 2,
      MESSAGE = 4,
      REASON = 1,
      CLIENT_COUNT = 3
    } type_t;

    typedef union payload {
      typedef uint16_t client_count_t;

     private:
      char username[SBCP_MAX_USERNAME_LENGTH];
      char message[SBCP_MAX_MESSAGE_LENGTH];
      char reason[SBCP_MAX_REASON_LENGTH];
      client_count_t client_count;
      friend class Attribute;
    } payload_t;

    typedef uint16_t length_t;

   private:
    type_t type;
    length_t length;
    payload_t payload;

   public:
    explicit constexpr Attribute(type_t type) noexcept
        : type(type), length(0), payload() {}
    explicit Attribute(type_t, const char*);
    explicit Attribute(type_t, const char*, size_t);
    explicit Attribute(type_t, payload_t::client_count_t);
    constexpr type_t get_type() const noexcept { return type; }
    constexpr length_t get_length() const noexcept { return length; }
    constexpr size_t size() const noexcept {
      return sizeof(type) + sizeof(length) + length;
    }
    const char* get_username() const noexcept { return payload.username; }
    const char* get_message() const noexcept { return payload.message; }
    const char* get_reason() const noexcept { return payload.reason; }
    payload_t::client_count_t get_client_count() const noexcept {
      return payload.client_count;
    }

   private:
    void validate() const;
    void set_username(const char*, size_t) noexcept;
    void set_message(const char*, size_t) noexcept;
    void set_reason(const char*, size_t) noexcept;
    void set_client_count(payload_t::client_count_t) noexcept;
    friend class Message;
  } attribute_t;

  typedef enum class Type : uint8_t {
    JOIN = 2,
    SEND = 4,
    FWD = 3,
    ACK = 7,
    NAK = 5,
    ONLINE = 8,
    OFFLINE = 6,
    IDLE = 9
  } type_t;

  typedef uint16_t version_t;
  typedef uint16_t length_t;

 private:
  version_t version : 9;
  type_t type : 7;
  length_t length;
  char payload[SBCP_MAX_PAYLOAD_LENGTH];

 public:
  constexpr Message(type_t type) noexcept
      : version(SBCP_VERSION), type(type), length(0), payload() {}
  void validate() const;
  void validate_version() const;
  type_t constexpr get_type() const noexcept { return type; }
  length_t constexpr get_length() const noexcept { return length; }

  void add_attribute(const attribute_t&);
  void add_attribute(attribute_t::type_t, const char*, size_t);
  void add_attribute(attribute_t::type_t, const char*);
  void add_attribute(attribute_t::type_t,
                     attribute_t::payload_t::client_count_t);

  const attribute_t& operator[](size_t) const;

 private:
  typedef class AttributeIterator {
   public:
    typedef std::input_iterator_tag iterator_category;
    typedef attribute_t value_type;
    typedef ptrdiff_t difference_type;
    typedef attribute_t* pointer;
    typedef attribute_t& reference;

    explicit AttributeIterator(const Message* msg) : msg(msg), offset(0) {
      msg->validate();
    }
    explicit AttributeIterator(const Message* msg, size_t offset)
        : msg(msg), offset(offset) {
      msg->validate();
    }

    reference operator*() {
      reference ref =
          *reinterpret_cast<pointer>(const_cast<char*>(msg->payload + offset));
      ref.validate();
      return ref;
    }
    pointer operator->() { return &operator*(); }
    AttributeIterator& operator++() {
      offset += operator*().size();
      return *this;
    }
    bool operator==(const AttributeIterator& other) const {
      return msg == other.msg && offset == other.offset;
    }
    bool operator!=(const AttributeIterator& other) const {
      return !(*this == other);
    }

   private:
    const Message* msg;
    size_t offset;
  } iterator;

  typedef const iterator const_iterator;

 public:
  iterator begin() { return iterator(this); }
  iterator end() { return iterator(this, length); }
  const_iterator begin() const { return const_iterator(this); }
  const_iterator end() const { return const_iterator(this, length); }

} message_t;

typedef class Message::Attribute attribute_t;
typedef enum Message::Attribute::Type attribute_type_t;
typedef enum Message::Type message_type_t;

}  // namespace sbcp

#endif