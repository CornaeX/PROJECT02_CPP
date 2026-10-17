#ifndef CAMPUS_TOUR_ITINERARY_LINKED_LIST_H
#define CAMPUS_TOUR_ITINERARY_LINKED_LIST_H

#include "CampusTree.h"

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

struct ItineraryMatch {
    std::size_t position{};
    std::string locationId;
    std::string locationName;
    std::string nextLocationName;
};

class ItineraryLinkedList {
public:
    ItineraryLinkedList() = default;
    ItineraryLinkedList(const ItineraryLinkedList&) = delete;
    ItineraryLinkedList& operator=(const ItineraryLinkedList&) = delete;

    void create();
    bool empty() const;
    std::size_t size() const;

    bool insert(const std::string& locationId, std::size_t oneBasedPosition,
                const CampusTree& tree, std::string& error);
    bool deleteAt(std::size_t oneBasedPosition, std::string& error);
    bool deleteById(const std::string& locationId, std::string& error);
    std::size_t removeAllById(const std::string& locationId);

    std::vector<ItineraryMatch> search(const std::string& query, const CampusTree& tree) const;
    std::vector<std::string> locationIds() const;
    void display(const CampusTree& tree, std::ostream& out) const;

private:
    struct Node {
        std::string locationId;
        std::unique_ptr<Node> next;
        explicit Node(std::string id) : locationId(std::move(id)) {}
    };

    std::unique_ptr<Node> head_;
    Node* tail_ = nullptr; // non-owning pointer; head_/next own every node
    std::size_t size_ = 0;
};

#endif
