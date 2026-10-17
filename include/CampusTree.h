#ifndef CAMPUS_TOUR_CAMPUS_TREE_H
#define CAMPUS_TOUR_CAMPUS_TREE_H

#include "Place.h"

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

enum class TreeNodeType { Root, Zone, Category, Location };

struct TreeNode {
    TreeNodeType type;
    std::string label;
    Place place;
    std::vector<std::unique_ptr<TreeNode>> children;

    TreeNode(TreeNodeType nodeType, std::string nodeLabel);
};

class CampusTree {
public:
    CampusTree() = default;
    CampusTree(const CampusTree&) = delete;
    CampusTree& operator=(const CampusTree&) = delete;
    CampusTree(CampusTree&&) noexcept = default;
    CampusTree& operator=(CampusTree&&) noexcept = default;

    bool initializeFromTsv(const std::string& filePath, std::string& error);
    bool initializeWithPlaces(const std::vector<Place>& places, std::string& error);
    bool saveToTsv(const std::string& filePath, std::string& error) const;
    bool initialized() const;

    bool insertPlace(const Place& place, std::string& error);
    bool deletePlaceById(const std::string& id, std::string& error);
    bool deleteEmptyCategory(const std::string& zone, const std::string& category, std::string& error);

    const Place* findById(const std::string& id) const;
    std::vector<Place> searchByName(const std::string& query) const;
    std::vector<Place> allPlaces() const;
    void displayHierarchy(std::ostream& out) const;

private:
    std::unique_ptr<TreeNode> root_;
    bool initialized_ = false;

    static std::unique_ptr<TreeNode> makeDefaultHierarchy();
    static TreeNode* findChild(TreeNode* parent, TreeNodeType type, const std::string& label);
    static const TreeNode* findChild(const TreeNode* parent, TreeNodeType type, const std::string& label);
    static void collectPlaces(const TreeNode* node, std::vector<Place>& output);
    static const Place* findByIdRecursive(const TreeNode* node, const std::string& id);
    static void searchNameRecursive(const TreeNode* node, const std::string& query, std::vector<Place>& output);
    static bool deletePlaceRecursive(TreeNode* node, const std::string& id);
    static void displayRecursive(const TreeNode* node, std::ostream& out, int depth);
};

#endif
