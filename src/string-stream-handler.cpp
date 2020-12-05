#include <string>
#include <cstring>
#include <sstream>
#include <iostream>

#include <handler_interface.h>

#if HANDLER_INTERFACE_VERSION_MAJOR != 1
#error "Handler requires interface version 1"
#endif

struct Download {
    std::string m_data;

    void push(const AsaData* data) {
        m_data.insert(m_data.end(), data->data, data->data + data->size);
    }

    AsaData* download() {
        const auto newline = m_data.find('\n');
        if (newline == std::string::npos) {
            return make_data_eoi();
        }

        const auto data = m_data.substr(0, newline);
        m_data.erase(0, newline+1);

        std::stringstream ss{data};
        float timestamp;
        ss >> timestamp;
        if (ss.fail()) {
            return make_data_fatal("Invalid line format");
        }
        while (ss.peek() == ' ') {
            ss.get();
        }
        const std::string value{std::istreambuf_iterator<char>{ss}, {}};

        return make_data_normal(timestamp, value);
    }

    static AsaData* make_data_normal(float time, const std::string& str) {
        const auto data_size = sizeof(uint32_t) + str.size();

        auto data = new AsaData {
            ASA_STATUS_NORMAL,
            time,
            data_size,
            new uint8_t[data_size],
            nullptr
        };

        AsaStringData* string_data = reinterpret_cast<AsaStringData*>(data->data);
        string_data->length = str.size();
        std::copy(str.begin(), str.end(), string_data->data);

        return data;
    }

    static AsaData* make_data_fatal(const char* error) {
        char* error_str = new char[strlen(error) + 1];
        strcpy(error_str, error);
        return new AsaData{ASA_STATUS_FATAL, 0, 0, nullptr, error_str};
    }

    static AsaData* make_data_again() {
        return new AsaData{ASA_STATUS_AGAIN};
    }

    static AsaData* make_data_eoi() {
        return new AsaData{ASA_STATUS_EOI};
    }

    static void free(AsaData* data) {
        delete [] data->data;
        delete [] data->error;
        delete data;
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

    int asa_handler_push(Download* self, const AsaData* data) {
        self->push(data);
        return 0;
    }

    AsaValueType asa_handler_get_type(Download*) {
        return ASA_STRING;
    }

    AsaData* asa_handler_download(Download* self) {
        return self->download();
    }

    AsaData* asa_handler_upload(void*) {
        return Download::make_data_fatal("Uploading is not supported");
    }

    void asa_handler_free(void*, AsaData* data) {
        Download::free(data);
    }
}
