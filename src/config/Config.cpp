#include "../../include/Config/Config.hpp"
#include "../../include/Config/Deckastore.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <random>

namespace fs = std::filesystem;
using std::ifstream;
using std::string;

bool rerun_oobe = false;

fs::path get_cfg_dir() {
    return fs::path(getenv(CFG_VAR)) / CFG_REQ;
}

uint64_t generate_uuid() {
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<uint64_t> distributor(1, UINT64_MAX);

    return distributor(generator);
}

int check_config() {
    Deckastore& dxstore = Deckastore::get();
    fs::path dkd_dir = get_cfg_dir();

    if (!fs::exists(dkd_dir / "config.json") || rerun_oobe) {
        rerun_oobe = false;
        if (run_oobe()) return -1;
    }

    ifstream reader(dkd_dir / "config.json");
    json config = json::parse(reader);
    reader.close();

    if (config.contains("mode") && config.contains("server") && config["server"].is_object()) {
        if (config["mode"].is_number_integer() && (config["mode"] == 1 ||
                                                   !static_cast<int>(config["mode"]))) {
            dxstore.set_mode(config["mode"]);
        } else {
            rerun_oobe = true;
            return check_config();
        }
        if (config["server"].contains("discoverable")) {
            if (config["server"]["discoverable"].is_boolean()) {
                dxstore.set_discoverable(config["server"]["discoverable"]);
            } else {
                rerun_oobe = true;
                return check_config();
            }
        } else {
            rerun_oobe = true;
            return check_config();
        }
        if (config["server"].contains("port")) {
            dxstore.set_port(config["server"]["port"].get<uint16_t>());
        }
    } else {
        rerun_oobe = true;
        return check_config();
    }
    dxstore.config = config;
    return 0;
}
