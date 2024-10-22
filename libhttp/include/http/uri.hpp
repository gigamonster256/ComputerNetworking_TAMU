#pragma once

#include <optional>
#include <string>
#include <variant>

namespace http {

// URI = ( absoluteURI | relativeURI ) [ "#" fragment ]
// absoluteURI = scheme ":" *( uchar | reserved )
// relativeURI = net_path | abs_path | rel_path

class URI {
  typedef struct {
    std::string scheme;
    std::string path;
  } AbsoluteURI;
  typedef std::string RelativeURI;

  std::variant<AbsoluteURI, RelativeURI> uri;
  std::optional<std::string> fragment;

 public:
  URI() = default;
  explicit URI(const std::string& uri);

  std::string to_string() const;
  const std::string& get_path() const;
};

}  // namespace http