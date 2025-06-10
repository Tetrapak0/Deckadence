#include "../../include/GUI/GUI.hpp"
#include "../../include/GUI/Button.hpp"
#include "../../include/Header.hpp"

#include <any>
#include <cstdint>

namespace NxSh::GUI {

void Item::set_label(string label) {
    this->label = label;
}

string Item::get_label() {
    return this->label;
}

/* this should be a "one size fits all" function, however it does not handle:
 * - pointer data types
 * - structs / classes / unions, etc.
 * - any complex std:: (vector, map/unordered_map, etc.)

 * Implement them yourself should you need them
 */
int Item::show_properties() {
    static unordered_map<string, std::any> sms(this->members);
    for (auto& member : this->members) {
        ImGui::Text("%s", member.first.c_str());
        ImGui::SameLine();

        const type_info& ti = member.second.type();
        const string label = "##" + member.first;
        if (ti == typeid(string)) {
            ImGui::InputText(label.c_str(), &std::any_cast<string&>(member.second));
        } else if (ti == typeid(int8_t)) {
            ImGui::InputScalar(label.c_str(), ImGuiDataType_S8, &std::any_cast<int8_t&>(member.second));
        } else if (ti == typeid(int16_t)) {
            ImGui::InputScalar(label.c_str(), ImGuiDataType_S16, &std::any_cast<int16_t&>(member.second));
        } else if (ti == typeid(int32_t)) {
            ImGui::InputScalar(label.c_str(), ImGuiDataType_S32, &std::any_cast<int32_t&>(member.second));
        } else if (ti == typeid(int64_t)) {
            ImGui::InputScalar(label.c_str(), ImGuiDataType_S64, &std::any_cast<int64_t&>(member.second));
        } else if (ti == typeid(uint8_t)) {
            ImGui::InputScalar(label.c_str(), ImGuiDataType_U8, &std::any_cast<uint8_t&>(member.second));
        } else if (ti == typeid(uint16_t)) {
            ImGui::InputScalar(label.c_str(), ImGuiDataType_U16, &std::any_cast<uint16_t&>(member.second));
        } else if (ti == typeid(uint32_t)) {
            ImGui::InputScalar(label.c_str(), ImGuiDataType_U32, &std::any_cast<uint32_t&>(member.second));
        } else if (ti == typeid(uint64_t)) {
            ImGui::InputScalar(label.c_str(), ImGuiDataType_U64, &std::any_cast<uint64_t&>(member.second));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text("Error while parsing member variable \"%s\" with type_info::name() \"%s\":\n"
                         "Type not supported by built in properties function.\n"
                         "Please override it with your custom function to use this type.\n", // TODO: HEY HEY IMPLEMENT FOLDER HIERARCHY IN FOLDER PROPERTIES!!1!1!!1!!!1!11
                         member.first.c_str(), ti.name());
            ImGui::PopStyleColor();
        }
    }
}

void Item::set_member(string name, std::any value) {
    // The developer should check and make sure they're the same type; that's not my job
    // ... also, what if the type can be changed? Though that's up to the developer.
    // alternatively just implement another function. it's virtual after all
    this->members[name] = value;
}

std::any Item::get_member(string name) {
    return members[name];
}

int Item::get_dtype() {
    return this->uses_thumbnail;
}

void Item::set_dtype(int dtype) {
    this->uses_thumbnail = dtype;
}

int FileButton::execute() {
    string path = std::any_cast<string>(this->members["path"]);
    if (!path.empty()) {
        string cmd = "\"" + path +"\"";
        LOG(stdout, "Opening: %s\n", cmd.c_str());
#ifdef _WIN32
        ShellExecuteA(NULL, "open", cmd.c_str(), NULL, NULL, SW_SHOW);
#else

#endif
    } // TODO: POSIX execution
}

int Folder::show_properties() {
    // This does nothing (for now)
    return 0;
}
}
