#include <string>
#include <cstring>
#include <sstream>
#include <iostream>

extern "C" {
#include <asampl-ffi/ffi.h>
}

#if ASAMPL_FFI_VERSION_MAJOR != 0
#error "Handler requires interface version 0"
#endif

struct Download {
    std::string m_data;

    void push(const AsaBytes* data) {
        m_data.insert(m_data.end(), data->data, data->data + data->size);
    }

    AsaHandlerResponse download() {
        AsaHandlerResponse response;

        const auto newline = m_data.find('\n');
        if (newline == std::string::npos) {
            asa_new_response_eoi(&response);
            return response;
        }

        const auto data = m_data.substr(0, newline);
        m_data.erase(0, newline+1);

        std::stringstream ss{data};
        double timestamp;
        ss >> timestamp;
        if (ss.fail()) {
            asa_new_response_fatal("Invalid line format", &response);
            return response;
        }
        while (ss.peek() == ' ') {
            ss.get();
        }
        const std::string value{std::istreambuf_iterator<char>{ss}, {}};

        AsaValueContainer* container = asa_alloc_container();
        asa_new_string_copy(ss.str().data(), ss.str().size(), container);
        container->timestamp = timestamp;
        asa_new_response_normal(container, &response);
        return response;
    }
};

extern "C" {
    Download* asa_handler_open_download() {
        return new Download;
    }

    void* asa_handler_open_upload() {
        return nullptr;
    }

    void asa_handler_close(Download* self) {
        delete self;
    }

    int asa_handler_push(Download* self, const AsaBytes* data) {
        self->push(data);
        return 0;
    }

    AsaHandlerResponse asa_handler_download(Download* self) {
        return self->download();
    }

    AsaHandlerResponse asa_handler_upload(void*) {
        AsaHandlerResponse response;
        asa_new_response_fatal("Uploading is not supported", &response);
        return response;
    }
}
