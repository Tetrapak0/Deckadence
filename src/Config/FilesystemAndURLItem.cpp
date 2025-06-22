#include "../../include/Config/Config.hpp"
#include "../../include/Config/Deckastore.hpp"
#include "../../include/Config/Item.hpp"
#include "../../include/GUI/GUI.hpp"
#include "../../include/Utilities/FileDialog.hpp"

FilesystemAndURLItem::FilesystemAndURLItem(json* config, Profile& parent_profile, FolderItem* parent) : Item(config, parent_profile, parent) {
    if (config) {
        if (config->contains("label") && (*config)["label"].is_string()) {
            this->label = (*config)["label"].get<string>();
            this->m_label = label;
        }
        if (config->contains("path") && (*config)["path"].is_string()) {
            this->path = (*config)["path"].get<string>();
            this->m_path = path;
        }
    }
}

void FilesystemAndURLItem::draw_properties() {
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Path: ");
    ImGui::SameLine();
    ImGui::InputText("##cmd", &this->m_path);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetWindowWidth()-ImGui::GetCursorPosX()-12);
    if (ImGui::BeginCombo("##browse", "Browse")) {
        if (ImGui::Selectable("File")) {
            string res = nfd_open_file();
            if (!res.empty())
                this->m_path = res;
        }
        if (ImGui::Selectable("Directory")) {
            string res = nfd_open_dir();
            if (!res.empty())
                this->m_path = res;
        }
        ImGui::EndCombo();
    }
#ifdef _WIN32
    ImGui::BeginDisabled();
    ImGui::TextWrapped("(i) If you cannot see some shortcuts, try looking at the public desktop");
    ImGui::EndDisabled();
#endif
}

void FilesystemAndURLItem::properties_cancel() {
    this->m_label = label;
    this->m_path = path;
}

void FilesystemAndURLItem::properties_apply() {
    this->label = this->m_label;
    this->path = this->m_path;
    int idx = Deckastore::get().draw_item_properties;
    if (!config)
        config = &this->parent_profile.root->request_config(idx);
    else
        this->config->clear();
    (*this->config)["idx"] = idx;
    (*this->config)["label"] = this->label;
    (*this->config)["type"] = this->get_typename();
    (*this->config)["path"] = this->path;
    this->parent_profile.parent.update_config();
}

bool FilesystemAndURLItem::prop_apply_disable() {
    return this->m_path.empty();
}

void FilesystemAndURLItem::execute() {
#ifdef _WIN32
    ShellExecuteA(nullptr, "open", this->path.c_str(), nullptr, nullptr, SW_SHOW);
#else
    pid_t pid = fork();
    if (pid) {
        return;
    }

    setsid();

    int fd = open("/dev/null", O_RDWR);
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    if (fd > 2)
        close(fd);

    execlp("xdg-open", "xdg-open", this->path.c_str(), static_cast<char*>(nullptr));

    _exit(0);
#endif
}
