#include <array>
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include "dji/dji_protocol_data_structures.h"
#include "dji/dji_protocol_parser.h"
#include "dji/enums_logic.h"
#include "thread_safe_queue.hpp"

#include <simpleble/SimpleBLE.h>

struct CommandResult {
    void *structure;
    size_t length; // 结构体内存长度， 不包含 CmdSet 和 CmdId
};

class OsmoDevice {
public:
    OsmoDevice(std::string mac, SimpleBLE::Peripheral device) {
        parse_mac(mac);

        device_ = device;
        device_.connect();
        std::cout << "device mtu is " << device_.mtu();

        // 找到所有的 UUID
        std::vector<std::pair<SimpleBLE::BluetoothUUID, SimpleBLE::BluetoothUUID>> uuids;
        for (auto service : device_.services()) {
            for (auto characteristic : service.characteristics()) {
                uuids.push_back(std::make_pair(service.uuid(), characteristic.uuid()));
            }
        }

        // 打印所有的 UUID
        std::cout << "The following services and characteristics were found:" << std::endl;
        for (size_t i = 0; i < uuids.size(); i++) {
            std::cout << "[" << i << "] " << uuids[i].first << " " << uuids[i].second << std::endl;
        }

        // 订阅 NOTIFY
        for (auto uuid : uuids) {
            if (uuid.first.find("fff0") != std::string::npos && uuid.second.find("fff4") != std::string::npos) {
                service_uuid_ = uuid.first;
                notify_uuid_ = uuid.second;
                std::cout << "Found notify service and characteristic" << std::endl;
                std::cout << "Service UUID: " << service_uuid_ << std::endl;
            }
            if (uuid.first.find("fff0") != std::string::npos && uuid.second.find("fff5") != std::string::npos) {
                service_uuid_ = uuid.first;
                write_uuid_ = uuid.second;
                std::cout << "Found write service and characteristic" << std::endl;
                std::cout << "Service UUID: " << service_uuid_ << std::endl;
            }
        }

        device_.notify(service_uuid_, notify_uuid_, [this](SimpleBLE::ByteArray data) { osmo_notify_callback(data); });
    }
    ~OsmoDevice() {
        if (device_.is_connected()) {
            device_.disconnect();
        }
    }

    uint16_t get_seq() {
        uint16_t seq = seq_.load();
        seq_++;
        return seq;
    }

    void request_connect();
    void parse_mac(std::string mac);
    CommandResult send_command(uint8_t cmd_set, uint8_t cmd_id, uint8_t cmd_type, const void *structure, uint16_t seq);

    void osmo_notify_callback(SimpleBLE::ByteArray data) {
        if (data[0] != 0xAA) {
            std::cout << "notify data is not start with 0xAA" << std::endl;
            return;
        }

        notify_queue_.push(data);
    }

private:
    std::string service_uuid_ = "";
    std::string notify_uuid_ = "";
    std::string write_uuid_ = "";

    std::array<int8_t, 6> adapter_mac_;

    std::atomic<uint16_t> seq_ = 1;
    ThreadSafeQueue<SimpleBLE::ByteArray> notify_queue_;

    /**
     * @brief 链接状态信息
     * 0 - 未链接
     * 1 - 链接中
     * 2 - 链接成功
     * -1 - 链接失败
     */
    std::atomic<uint32_t> connect_status_ = 0;
    uint16_t verify_data_ = 0;

    SimpleBLE::Peripheral device_;
};

int main(int argc, char **argv) {
    if (!SimpleBLE::Adapter::bluetooth_enabled()) {
        std::cout << "Bluetooth is not enabled" << std::endl;
        return 1;
    }

    auto adapters = SimpleBLE::Adapter::get_adapters();
    if (adapters.empty()) {
        std::cout << "No Bluetooth adapters found" << std::endl;
        return 1;
    }

    // Use the first adapter
    auto adapter = adapters[0];

    // Do something with the adapter
    std::cout << "Adapter identifier: " << adapter.identifier() << std::endl;
    std::cout << "Adapter address: " << adapter.address() << std::endl;

    // Scan for devices
    adapter.scan_for(5000); // Scan for 10 seconds
    std::vector<SimpleBLE::Peripheral> devices = adapter.scan_get_results();

    SimpleBLE::Peripheral osmo;

    for (auto &device : devices) {
        std::cout << "Device name: " << device.identifier() << std::endl;
        std::cout << "Device address: " << device.address() << std::endl;

        // if name contains OsmoAction
        if (device.identifier().find("OsmoAction") != std::string::npos) {
            osmo = device;
            std::cout << "Found OsmoAction device" << std::endl;
            break;
        }
    }

    OsmoDevice osmo_device(adapter.address(), osmo);
    osmo_device.request_connect();

    while (true) {
        // sleep 1 second
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}

void OsmoDevice::request_connect() {
    // 构造连接请求
    uint16_t seq = get_seq();
    uint8_t verify_mode = 0;
    // 随机验证数据
    verify_data_ = (uint16_t)(rand() % 10000);
    connection_request_command_frame connection_request = {
        .device_id = 0xFF33,
        .mac_addr_len = (uint8_t)adapter_mac_.size(),
        .fw_version = 0,
        .verify_mode = verify_mode,
        .verify_data = verify_data_,
    };
    std::memcpy(connection_request.mac_addr, adapter_mac_.data(), adapter_mac_.size());

    CommandResult result = send_command(0x00, 0x19, CMD_WAIT_RESULT, &connection_request, seq);
    if (result.structure == nullptr) {
        std::cout << "Failed to send connection request" << std::endl;
        connect_status_ = -1;
        return;
    }

    {
        connection_request_response_frame *connection_response = (connection_request_response_frame *)result.structure;
        if (connection_response->device_id != 0xFF44 || connection_response->ret_code != 0) {
            std::cout << "Failed to connect to device" << std::endl;
            connect_status_ = -1;
            return;
        }
    }

    uint16_t recieved_seq = 0;
    // 等待相机发来连接消息
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        SimpleBLE::ByteArray data;
        notify_queue_.pop(data);
        protocol_frame_t frame;
        int ret = protocol_parse_notification(data.data(), data.size(), &frame);
        if (ret != 0) {
            std::cout << "Failed to parse notification" << std::endl;
            continue;
        }
        // 解析数据
        // cmd_set 0x00, cmd_id 0x19, cmd_type 0x22, 解析到的数据类型为 connection_request_response_frame
        if (frame.data[0] != 0x00 || frame.data[1] != 0x19) {
            std::cout << "Invalid command set or command id" << std::endl;
            continue;
        }
        size_t structure_data_length;
        void *structure_data =
            protocol_parse_data(frame.data, frame.data_length, frame.cmd_type, &structure_data_length);
        // verify_data 为随机数， verify_mode 为 2
        connection_request_command_frame *connection_request = (connection_request_command_frame *)structure_data;
        if (connection_request->verify_mode != 2) {
            std::cout << "Invalid verify data or verify mode" << std::endl;
            continue;
        }

        recieved_seq = frame.seq;
        std::cout << "connection request from: 0x" << std::hex << connection_request->device_id << std::dec
                  << std::endl;
        break;
    }

    // 最后一步，发送返回消息
    connection_request_response_frame connection_response = {.device_id = 0xFF33, .ret_code = 0};
    memset(connection_response.reserved, 0, sizeof(connection_response.reserved));

    send_command(0x00, 0x19, ACK_NO_RESPONSE, &connection_response, recieved_seq);
    connect_status_ = 1;
}

void OsmoDevice::parse_mac(std::string mac) {
    // six hex digits, separated by colons, e.g. "00:11:22:33:44:55"
    std::stringstream ss(mac);
    std::string token;
    std::array<int8_t, 6> mac_bytes;
    int i = 0;
    while (std::getline(ss, token, ':')) {
        mac_bytes[i] = std::stoi(token, nullptr, 16);
        i++;
    }
    adapter_mac_ = mac_bytes;
}

CommandResult OsmoDevice::send_command(uint8_t cmd_set, uint8_t cmd_id, uint8_t cmd_type, const void *structure,
                                       uint16_t seq) {
    CommandResult result = {NULL, 0};

    size_t frame_length = 0;
    uint8_t *frame = protocol_create_frame(cmd_set, cmd_id, cmd_type, structure, seq, &frame_length);
    if (frame == nullptr) {
        std::cout << "Failed to create frame" << std::endl;
        return result;
    }

    // 发送命令
    SimpleBLE::ByteArray data(frame, frame_length);

    std::cout << "Sending command: " << data.toHex() << std::endl;
    device_.write_command(service_uuid_, write_uuid_, data);

    void *structure_data = NULL;
    size_t structure_data_length = 0;

    switch (cmd_type) {
    case CMD_NO_RESPONSE:
    case ACK_NO_RESPONSE: {
        return result;
    }
    case CMD_RESPONSE_OR_NOT:
    case ACK_RESPONSE_OR_NOT: {
        protocol_frame_t frame;
        int ret = protocol_parse_notification(data.data(), data.size(), &frame);
        if (ret != 0) {
            std::cout << "Failed to parse notification" << std::endl;
            return result;
        }

        if (frame.seq != seq) {
            std::cout << "seq is not match, expected " << seq << ", got " << frame.seq << std::endl;
            return result;
        }

        structure_data = protocol_parse_data(frame.data, frame.data_length, frame.cmd_type, &structure_data_length);
        if (structure_data == nullptr) {
            std::cout << "Failed to parse data" << std::endl;
            return result;
        }
        result.structure = structure_data;
        result.length = structure_data_length;
        return result;
    }
    case CMD_WAIT_RESULT:
    case ACK_WAIT_RESULT: {
        // 解析 notify 数据，直到解析到 seq 对应的结果
        while (true) {
            SimpleBLE::ByteArray data;
            notify_queue_.pop(data);

            protocol_frame_t frame;
            int ret = protocol_parse_notification(data.data(), data.size(), &frame);
            if (ret != 0) {
                std::cout << "Failed to parse notification" << std::endl;
                return result;
            }

            if (frame.seq != seq) {
                std::cout << "seq is not match, expected " << seq << ", got " << frame.seq << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            structure_data = protocol_parse_data(frame.data, frame.data_length, frame.cmd_type, &structure_data_length);
            if (structure_data == nullptr) {
                std::cout << "Failed to parse data" << std::endl;
                return result;
            }

            result.structure = structure_data;
            result.length = structure_data_length;
            break;
        }
        return result;
    }
    default:
        return result;
    }
}
