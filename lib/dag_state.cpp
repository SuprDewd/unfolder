#include "dag_state.h"

State::State() {
    this->https = false;
    this->host = "";
    this->path.push_back('/');
}
