#include "CampusTree.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <unordered_set>

TreeNode::TreeNode(TreeNodeType nodeType, std::string nodeLabel)
    : type(nodeType), label(std::move(nodeLabel)) {}

namespace {
std::vector<std::string> splitTsv(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;
    while (std::getline(stream, field, '\t')) fields.push_back(field);
    if (!line.empty() && line.back() == '\t') fields.emplace_back();
    return fields;
}

bool parseDoubleStrict(const std::string& input, double& value) {
    try {
        std::size_t used = 0;
        value = std::stod(input, &used);
        return used == input.size();
    } catch (...) {
        return false;
    }
}

void addZoneAndCategories(TreeNode* root) {
    auto addZone = [root](const std::string& zoneName,
                          const std::vector<std::string>& categories) {
        auto zone = std::make_unique<TreeNode>(TreeNodeType::Zone, zoneName);
        for (const auto& category : categories) {
            zone->children.push_back(std::make_unique<TreeNode>(TreeNodeType::Category, category));
        }
        root->children.push_back(std::move(zone));
    };

    addZone("Campus-wide", {"University"});
    addZone("Central Campus", {"Education", "Student Services", "University Services", "Retail and Dining"});
    addZone("Medical Campus", {"Healthcare", "Medical Education", "University Services"});
    addZone("Sports Campus", {"Sports and Recreation"});
}
}

std::unique_ptr<TreeNode> CampusTree::makeDefaultHierarchy() {
    auto root = std::make_unique<TreeNode>(TreeNodeType::Root, "Naresuan University");
    addZoneAndCategories(root.get());
    return root;
}

TreeNode* CampusTree::findChild(TreeNode* parent, TreeNodeType type, const std::string& label) {
    if (!parent) return nullptr;
    for (auto& child : parent->children) {
        if (child->type == type && child->label == label) return child.get();
    }
    return nullptr;
}

const TreeNode* CampusTree::findChild(const TreeNode* parent, TreeNodeType type, const std::string& label) {
    if (!parent) return nullptr;
    for (const auto& child : parent->children) {
        if (child->type == type && child->label == label) return child.get();
    }
    return nullptr;
}

bool CampusTree::initializeFromTsv(const std::string& filePath, std::string& error) {
    if (initialized_) {
        error = "Campus Tree is already initialized; refusing to duplicate the dataset.";
        return false;
    }

    std::ifstream input(filePath);
    if (!input) {
        error = "Could not open data file: " + filePath;
        return false;
    }

    std::vector<Place> places;
    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        if (trim(line).empty() || line[0] == '#') continue;
        const auto fields = splitTsv(line);
        if (fields.size() != 9) {
            error = "Invalid TSV row " + std::to_string(lineNumber) + ": expected 9 tab-separated fields.";
            return false;
        }
        Place place;
        place.id = trim(fields[0]);
        place.name = trim(fields[1]);
        place.category = trim(fields[2]);
        place.zone = trim(fields[3]);
        if (!parseDoubleStrict(trim(fields[4]), place.latitude) ||
            !parseDoubleStrict(trim(fields[5]), place.longitude)) {
            error = "Invalid latitude/longitude on TSV row " + std::to_string(lineNumber) + ".";
            return false;
        }
        place.description = trim(fields[6]);
        place.sourceUrl = trim(fields[7]);
        place.verificationStatus = trim(fields[8]);
        places.push_back(std::move(place));
    }

    return initializeWithPlaces(places, error);
}

bool CampusTree::initializeWithPlaces(const std::vector<Place>& places, std::string& error) {
    if (initialized_) {
        error = "Campus Tree is already initialized; refusing to duplicate the dataset.";
        return false;
    }

    auto candidateRoot = makeDefaultHierarchy();
    std::unordered_set<std::string> ids;
    for (const auto& place : places) {
        if (!validatePlace(place, error)) {
            error = "Invalid location '" + place.id + "': " + error;
            return false;
        }
        if (!ids.insert(toLowerAscii(place.id)).second) {
            error = "Duplicate location ID in initial dataset: " + place.id;
            return false;
        }
        const TreeNode* zoneNode = findChild(candidateRoot.get(), TreeNodeType::Zone, place.zone);
        const TreeNode* categoryNode = findChild(zoneNode, TreeNodeType::Category, place.category);
        if (!categoryNode) {
            error = "Unknown zone/category in dataset: '" + place.zone + "' / '" + place.category + "'.";
            return false;
        }
    }

    for (const auto& place : places) {
        TreeNode* zoneNode = findChild(candidateRoot.get(), TreeNodeType::Zone, place.zone);
        TreeNode* categoryNode = findChild(zoneNode, TreeNodeType::Category, place.category);
        auto locationNode = std::make_unique<TreeNode>(TreeNodeType::Location, place.name);
        locationNode->place = place;
        categoryNode->children.push_back(std::move(locationNode));
    }
    root_ = std::move(candidateRoot);
    initialized_ = true;
    return true;
}

bool CampusTree::saveToTsv(const std::string& filePath, std::string& error) const {
    if (!initialized_) {
        error = "Cannot save: Campus Tree has not been initialized.";
        return false;
    }
    std::ofstream output(filePath, std::ios::trunc);
    if (!output) {
        error = "Could not write data file: " + filePath;
        return false;
    }
    output << "# id\tname\tcategory\tzone\tlatitude\tlongitude\tdescription\tsource_url\tverification_status\n";
    output << std::setprecision(10);
    for (const auto& place : allPlaces()) {
        output << place.id << '\t' << place.name << '\t' << place.category << '\t' << place.zone << '\t'
               << place.latitude << '\t' << place.longitude << '\t' << place.description << '\t'
               << place.sourceUrl << '\t' << place.verificationStatus << '\n';
    }
    if (!output) {
        error = "A write error occurred while saving: " + filePath;
        return false;
    }
    return true;
}

bool CampusTree::initialized() const { return initialized_; }

bool CampusTree::insertPlace(const Place& place, std::string& error) {
    if (!initialized_ || !root_) { error = "Initialize the Campus Tree before inserting locations."; return false; }
    if (!validatePlace(place, error)) return false;
    if (findById(place.id)) { error = "Location ID already exists: " + place.id; return false; }
    TreeNode* zoneNode = findChild(root_.get(), TreeNodeType::Zone, place.zone);
    TreeNode* categoryNode = findChild(zoneNode, TreeNodeType::Category, place.category);
    if (!categoryNode) {
        error = "Parent zone/category does not exist. Use one of the categories shown in the Tree.";
        return false;
    }
    auto locationNode = std::make_unique<TreeNode>(TreeNodeType::Location, place.name);
    locationNode->place = place;
    categoryNode->children.push_back(std::move(locationNode));
    return true;
}

bool CampusTree::deletePlaceRecursive(TreeNode* node, const std::string& id) {
    if (!node) return false;
    for (auto it = node->children.begin(); it != node->children.end(); ++it) {
        if ((*it)->type == TreeNodeType::Location &&
            toLowerAscii((*it)->place.id) == toLowerAscii(id)) {
            // Erasing the owning unique_ptr safely destroys the selected leaf.
            node->children.erase(it);
            return true;
        }
        if (deletePlaceRecursive(it->get(), id)) return true;
    }
    return false;
}

bool CampusTree::deletePlaceById(const std::string& id, std::string& error) {
    if (!initialized_ || !root_) { error = "Initialize the Campus Tree before deleting locations."; return false; }
    if (!deletePlaceRecursive(root_.get(), trim(id))) {
        error = "No location found with ID: " + trim(id);
        return false;
    }
    return true;
}

bool CampusTree::deleteEmptyCategory(const std::string& zone, const std::string& category, std::string& error) {
    if (!initialized_ || !root_) { error = "Campus Tree is not initialized."; return false; }
    TreeNode* zoneNode = findChild(root_.get(), TreeNodeType::Zone, zone);
    if (!zoneNode) { error = "Zone does not exist: " + zone; return false; }
    for (auto it = zoneNode->children.begin(); it != zoneNode->children.end(); ++it) {
        if ((*it)->type == TreeNodeType::Category && (*it)->label == category) {
            if (!(*it)->children.empty()) {
                error = "Cannot delete a category that still contains locations.";
                return false;
            }
            zoneNode->children.erase(it);
            return true;
        }
    }
    error = "Category does not exist: " + category;
    return false;
}

const Place* CampusTree::findByIdRecursive(const TreeNode* node, const std::string& id) {
    if (!node) return nullptr;
    if (node->type == TreeNodeType::Location && toLowerAscii(node->place.id) == toLowerAscii(id)) {
        return &node->place;
    }
    for (const auto& child : node->children) {
        if (const Place* found = findByIdRecursive(child.get(), id)) return found;
    }
    return nullptr;
}

const Place* CampusTree::findById(const std::string& id) const {
    if (!initialized_ || !root_) return nullptr;
    return findByIdRecursive(root_.get(), trim(id));
}

void CampusTree::searchNameRecursive(const TreeNode* node, const std::string& query, std::vector<Place>& output) {
    if (!node) return;
    if (node->type == TreeNodeType::Location &&
        toLowerAscii(node->place.name).find(query) != std::string::npos) {
        output.push_back(node->place);
    }
    for (const auto& child : node->children) searchNameRecursive(child.get(), query, output);
}

std::vector<Place> CampusTree::searchByName(const std::string& query) const {
    std::vector<Place> output;
    if (!initialized_ || !root_) return output;
    const std::string normalized = toLowerAscii(trim(query));
    if (normalized.empty()) return output;
    searchNameRecursive(root_.get(), normalized, output);
    return output;
}

void CampusTree::collectPlaces(const TreeNode* node, std::vector<Place>& output) {
    if (!node) return;
    if (node->type == TreeNodeType::Location) output.push_back(node->place);
    for (const auto& child : node->children) collectPlaces(child.get(), output);
}

std::vector<Place> CampusTree::allPlaces() const {
    std::vector<Place> output;
    if (initialized_ && root_) collectPlaces(root_.get(), output);
    return output;
}

void CampusTree::displayRecursive(const TreeNode* node, std::ostream& out, int depth) {
    if (!node) return;
    out << std::string(static_cast<std::size_t>(depth * 2), ' ') << "- " << node->label;
    if (node->type == TreeNodeType::Location) {
        out << " [" << node->place.id << "]  (" << node->place.latitude << ", "
            << node->place.longitude << ")  " << node->place.verificationStatus;
    }
    out << '\n';
    for (const auto& child : node->children) displayRecursive(child.get(), out, depth + 1);
}

void CampusTree::displayHierarchy(std::ostream& out) const {
    if (!initialized_ || !root_) { out << "Campus Tree is empty. Choose option 1 to initialize it.\n"; return; }
    out << "CAMPUS TREE (root -> zone -> category -> location)\n";
    displayRecursive(root_.get(), out, 0);
}
