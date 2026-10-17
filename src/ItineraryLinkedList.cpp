#include "ItineraryLinkedList.h"

#include <ostream>

void ItineraryLinkedList::create() {
    head_.reset();
    tail_ = nullptr;
    size_ = 0;
}

bool ItineraryLinkedList::empty() const { return size_ == 0; }
std::size_t ItineraryLinkedList::size() const { return size_; }

bool ItineraryLinkedList::insert(const std::string& locationId, std::size_t oneBasedPosition,
                                 const CampusTree& tree, std::string& error) {
    if (!tree.findById(locationId)) {
        error = "Cannot add itinerary stop: location ID does not exist in the Campus Tree.";
        return false;
    }
    if (oneBasedPosition < 1 || oneBasedPosition > size_ + 1) {
        error = "Invalid position. Insert position must be between 1 and " + std::to_string(size_ + 1) + ".";
        return false;
    }

    auto node = std::make_unique<Node>(locationId);
    Node* rawNode = node.get();
    if (oneBasedPosition == 1) {
        node->next = std::move(head_);
        head_ = std::move(node);
        if (size_ == 0) tail_ = rawNode;
    } else if (oneBasedPosition == size_ + 1) {
        tail_->next = std::move(node);
        tail_ = rawNode;
    } else {
        Node* previous = head_.get();
        for (std::size_t position = 1; position < oneBasedPosition - 1; ++position) {
            previous = previous->next.get();
        }
        node->next = std::move(previous->next);
        previous->next = std::move(node);
    }
    ++size_;
    return true;
}

bool ItineraryLinkedList::deleteAt(std::size_t oneBasedPosition, std::string& error) {
    if (empty()) { error = "Itinerary is empty; there is no stop to delete."; return false; }
    if (oneBasedPosition < 1 || oneBasedPosition > size_) {
        error = "Invalid position. Itinerary currently has " + std::to_string(size_) + " stop(s).";
        return false;
    }

    if (oneBasedPosition == 1) {
        Node* oldHead = head_.get();
        auto removed = std::move(head_);
        head_ = std::move(removed->next);
        if (tail_ == oldHead) tail_ = nullptr;
    } else {
        Node* previous = head_.get();
        for (std::size_t position = 1; position < oneBasedPosition - 1; ++position) {
            previous = previous->next.get();
        }
        Node* oldNode = previous->next.get();
        auto removed = std::move(previous->next);
        previous->next = std::move(removed->next);
        if (tail_ == oldNode) tail_ = previous;
    }
    --size_;
    if (size_ == 0) tail_ = nullptr;
    return true;
}

bool ItineraryLinkedList::deleteById(const std::string& locationId, std::string& error) {
    std::size_t position = 1;
    for (Node* current = head_.get(); current; current = current->next.get(), ++position) {
        if (toLowerAscii(current->locationId) == toLowerAscii(trim(locationId))) {
            return deleteAt(position, error);
        }
    }
    error = "Location ID is not present in the itinerary: " + locationId;
    return false;
}

std::size_t ItineraryLinkedList::removeAllById(const std::string& locationId) {
    std::size_t removedCount = 0;
    std::string ignored;
    // Deleting by index is safe even when repeated itinerary stops reference the same ID.
    std::size_t position = 1;
    while (position <= size_) {
        Node* current = head_.get();
        for (std::size_t i = 1; i < position; ++i) current = current->next.get();
        if (toLowerAscii(current->locationId) == toLowerAscii(trim(locationId))) {
            if (deleteAt(position, ignored)) ++removedCount;
        } else {
            ++position;
        }
    }
    return removedCount;
}

std::vector<ItineraryMatch> ItineraryLinkedList::search(const std::string& query, const CampusTree& tree) const {
    std::vector<ItineraryMatch> matches;
    const std::string q = toLowerAscii(trim(query));
    if (q.empty()) return matches;
    std::size_t position = 1;
    for (Node* current = head_.get(); current; current = current->next.get(), ++position) {
        const Place* place = tree.findById(current->locationId);
        const std::string name = place ? place->name : "[MISSING LOCATION: " + current->locationId + "]";
        if (toLowerAscii(current->locationId) == q || toLowerAscii(name).find(q) != std::string::npos) {
            std::string nextName = "(end of itinerary)";
            if (current->next) {
                const Place* nextPlace = tree.findById(current->next->locationId);
                nextName = nextPlace ? nextPlace->name : "[MISSING LOCATION: " + current->next->locationId + "]";
            }
            matches.push_back({position, current->locationId, name, nextName});
        }
    }
    return matches;
}

std::vector<std::string> ItineraryLinkedList::locationIds() const {
    std::vector<std::string> ids;
    ids.reserve(size_);
    for (Node* current = head_.get(); current; current = current->next.get()) ids.push_back(current->locationId);
    return ids;
}

void ItineraryLinkedList::display(const CampusTree& tree, std::ostream& out) const {
    out << "ITINERARY LINKED LIST (head -> next -> ... -> null)\n";
    if (empty()) { out << "  (empty)\n"; return; }
    std::size_t position = 1;
    for (Node* current = head_.get(); current; current = current->next.get(), ++position) {
        const Place* place = tree.findById(current->locationId);
        out << "  " << position << ". ";
        if (place) out << place->name << " [" << place->id << "]";
        else out << "[MISSING LOCATION: " << current->locationId << "]";
        out << (current->next ? " ->\n" : " -> NULL\n");
    }
}
