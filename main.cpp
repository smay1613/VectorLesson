#include <iostream>
#include <vector>

struct SomeClass
{
    SomeClass() {
        std::cout << "Some class default constructed!" << std::endl;
    }
    SomeClass(size_t data) {
        std::cout << "Some class constructed with param " << std::to_string(data) << std::endl;
    }
    SomeClass(const SomeClass&) {
        std::cout << "Some class copy constructed" << std::endl;
    }
    SomeClass(SomeClass&&) {
        std::cout << "Some class move constructed" << std::endl;
    }
    ~SomeClass() {
        std::cout << "Some class destroyed" << std::endl;
    }
};

void investigateIteratorInvalidation()
{
    {
        std::vector<int> data {1, 2, 3, 4, 5};
        for (auto& element : data) { // invalidation in loop, very common error case
            data.push_back(element * 2);
            data.erase(data.begin() + element);
        }
    }
    {
        std::vector<int> data (5);
        auto p1 = data.begin();
        data.push_back(2);   // p1 may have been invalidated, since the capacity was unknown.

        data.reserve(20);    // Capacity is now at least 20.
        auto p2 = data.begin();
        data.push_back(4);   // p2 is *not* invalidated, since the size of `v` is now 7.
        data.insert(data.end(), 30, 9); // Inserts 30 elements at the end. The size exceeds the
        // requested capacity of 20, so `p2` is (probably) invalidated.
        auto p3 = data.begin();
        data.reserve(data.capacity() + 20); // Capacity exceeded, thus `p3` is invalid.
    }
    {
        std::vector<int> data(10);
        auto p1 = data.begin();
        auto p2 = data.begin() + 5;
        data.erase(data.begin() + 3, data.end()); // `p2` is invalid, but `p1` remains valid.
    }
}

void investigateReserve()
{
    std::vector<int> data;
    data.reserve(1000);

    for (int i = 0; i < 1000; ++i) { // will work faster, because preallocated
        data.push_back(i);
    }

    std::vector<int> data1;
    for (int i = 0; i < 1000; ++i) {
        data1.push_back(i);
    }
}


void investigateConstructors1()
{
    // CONSTRUCTORS
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        std::vector<int> data {}; // default constructor, empty container

        std::allocator<int> defaultAllocator {};
        std::vector<int, std::allocator<int>> data1 {defaultAllocator}; // default constructor, use allocator

        const size_t dataSize {42};
        // note: round braces!
        std::vector<int> data2 (dataSize); // initialized with 42 elements with default constructed objects of int
        std::cout << "Count default init size: " << data2.size() << std::endl;
        std::cout << "Any element is default initializated, "
                     "for example first element = " << data2[0] << std::endl;

        const int someData {1613};
        std::vector<int> data3 (dataSize, someData); // initialize with dataSize copies of someData
        std::cout << "n copies init size: " << data3.size() << std::endl;
        std::cout << "Any element is equal to someData value = " << data3[0] << std::endl;

        std::vector<int> data4 (dataSize, someData, defaultAllocator); // combined version: init with dataSize copies of someData, use allocator
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
void investigateConstructors2()
{
    // CONSTRUCTORS (continued)
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        const int someData {14};
        std::vector<int> data {someData}; // init from element, preferring initializer list constructor
        std::cout << "Element initialization, size: " << data.size() <<
                     " , first element: " << *data.cbegin() << std::endl;

        std::vector<int> data1 {data}; // copy constructor, copies elements and allocator from data to data1
        std::cout << "Copy initialization, size: " << data1.size() <<
                     " , first element: " << *data1.cbegin() << std::endl;

        std::vector<int> data2 {std::move(data1)};
        std::cout << "Move initialization, moved object size: " << data1.size() <<
                     " , moved object first element: " << ((data1.size() > 0) ? *data1.cbegin() : -1) << std::endl;

        std::cout << "Move initialization, object size: " << data2.size() <<
                     " , moved object first element: " << *data2.cbegin() << std::endl;

        std::allocator<int> defaultAllocator;
        std::vector<int> data3 {{someData, someData}, defaultAllocator}; // init from initializer_list

        std::vector<int> data4 {data3.begin(), data3.begin() + 1}; // init from sequence
        std::vector<int> data5 {data3.begin(), data3.begin() + 1, defaultAllocator}; // init from sequence + allocator

        // DESTRUCTOR
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        {
            const size_t dataSize {4};
            std::vector<SomeClass> data6 (dataSize);
            std::cout << "Waiting for " << dataSize << " destructions:" << std::endl;
            // vector becomes out of scope, so RAII will clear data
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void investigateAssignments()
{
    // Assignment
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        const int someData {42};
        std::vector<int> data {someData};

        std::vector<int> data1, data2, data3;

        data1 = data; // copy assignment
        data2 = std::move(data1); // move assignment
        data3 = {someData, someData}; // assign initializer list

        size_t count {10};
        data.assign(count, someData); // assign count copies of someData

        data.assign(data3.begin(), data3.begin() + 1); // assign sequence by iterators
        data.assign(&data3[0], &data3[1]); // assign sequence

        data.assign({someData, someData});

        //std::vector<std::string> data4;
        //data4.assign("Testing", "Testing2"); // Error: two pointers, that doesn't point on the same array
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
void investigateConstructors()
{
    investigateConstructors1();
    investigateConstructors2();
    investigateAssignments();
}

template<class T>
void printVector(const std::vector<T>& vectorToPrint)
{
    static const T defaultEntry {};
    for (const auto& entry : vectorToPrint) { // range for
        if (entry == defaultEntry) {
            std::cout << "<default entry>" << std::endl;
        } else {
            std::cout << entry << std::endl;
        }
    }
    std::cout << std::endl;
}

void investigateCapacityOperations(std::vector<std::string>& data)
{
    // Capacity operations
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::cout << "File list size: " << data.size() << std::endl;
    std::cout << "File list capacity: " << data.capacity() << std::endl;
    std::cout << "File list max size: " << data.max_size() << std::endl;

    if (data.empty()) { // check for size == 0
        std::cout << "File list is empty!" << std::endl;
    } else {
        std::cout << "File list is not empty. " << std::endl;
    }

    const size_t expectedSize {100};
    data.reserve(expectedSize); // reserve more
    data.reserve(expectedSize / 2); // reserve less

    investigateReserve();

    std::cout << "File list capacity after reserve: " << data.capacity() << std::endl <<
                 "and size: " << data.size() << std::endl;

    // NOTE: RESIZE IS A MODIFIER
    data.resize(5); // resize less
    std::cout << "File list capacity after resize to 5: " << data.capacity() << std::endl <<
                 "and size: " << data.size() << std::endl;
    std::cout << "After resizing to 5 vector contains: " << std::endl;
    printVector(data);

    data.resize(10); // resize more
    std::cout << "File list capacity after resize to 10: " << data.capacity() << std::endl
              << "and size: " << data.size() << std::endl;
    std::cout << "After resizing to 10 vector contains: " << std::endl;
    printVector(data);

    data.shrink_to_fit(); // deallocate unused space, capacity will equal to size
    std::cout << "File list capacity after shrink: " << data.capacity() << std::endl <<
                 "and size: " << data.size() << std::endl;
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void investigateAccessOperations(std::vector<std::string>& data)
{
    // Element access
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::cout << "Second element: " << data[1] << std::endl; // subscripting access, unchecked
    std::cout << "Second element: " << data.at(1) << std::endl; // checked access, if exeeds, throws std::out_of_range

    try {
        data.at(15);
    } catch (const std::out_of_range& ex) {
        std::cout << "Catched out of range!" << std::endl;
        std::cout << ex.what() << std::endl;
    }

    std::cout << "First element: " << data.front() << std::endl; // read only access to the first element
    std::cout << "Last element: " << data.back() << std::endl; // read only access to the last element

    data.data()[1] = "Be careful when working with raw data!";
    std::cout << "Second element: " << data.data()[1] << std::endl;
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void investigateModifiers(std::vector<std::string>& data)
{
    // Modifiers
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    data.clear(); // deletes the data
    data.shrink_to_fit(); // deallocate reserved
    // or std::vector<std::string>().swap(data);
    std::cout << "List cleared! It's size now is " << data.size() <<
                 " and capacity is " << data.capacity() << std::endl;
    const size_t filesCount {6};

    for (size_t i = 0; i < filesCount; ++i) {
        std::string fileName {std::to_string(i) + ".mp3"};
        data.push_back(fileName); // add one element to the end

        std::cout << "Appended [" << i + 1<< "]/[" << filesCount << "] " << std::endl;
        std::cout << "File list size: " << data.size() << std::endl
                  << "File list capacity: " << data.capacity() << std::endl;
    }

    std::cout << std::endl;
    std::cout << std::endl;

    std::vector<SomeClass> someData;
    const size_t someSize {6};
    for (size_t i = 0; i < someSize; ++i) {
        someData.emplace_back(i); // in-place adding one element to the end

        std::cout << "Emplaced [" << i + 1 << "]/[" << someSize << "] " << std::endl;
        std::cout << "File list size: " << someData.size() << std::endl
                  << "File list capacity: " << someData.capacity() << std::endl;
    }

    std::cout << std::endl;
    std::cout << std::endl;

    std::vector<SomeClass> someData2;
    for (size_t i = 0; i < someSize; ++i) {
        SomeClass someObject {i};
        someData2.push_back(someObject); // add one element to the end

        std::cout << "Pushed [" << i + 1 << "]/[" << someSize << "] " << std::endl;
        std::cout << "Some data 2 size: " << someData2.size() << std::endl
                  << "Some data 2 capacity: " << someData2.capacity() << std::endl;
    }

    std::cout << std::endl;
    std::cout << std::endl;

    for (size_t i = 0; i < filesCount / 2; ++i) {
        data.pop_back(); // remove one element at the end

        std::cout << "Popped [" << i + 1 << "]/[" << filesCount / 2 << "] " << std::endl;
        std::cout << "File list size: " << data.size() << std::endl
                  << "File list capacity: " << data.capacity() << std::endl;
    }

    std::cout << std::endl;
    std::cout << std::endl;

    std::cout << "Before insertion: " << std::endl;
    printVector(data);

    std::cout << "After insertion: " << std::endl;
    const std::string discoveredFile {"newfile.jpg"};
    data.insert(std::next(data.begin(), 2), discoveredFile); // adds an element to a specified position
    printVector(data);

    data.insert(std::next(data.begin(), 2), 5, discoveredFile); // adds 5 elements to a specified position
    printVector(data);

    std::cout << "After emplace insertion: " << std::endl;
    data.emplace(std::next(data.begin(), 3), "newfile.png"); // inserts inplace without copy
    printVector(data);

    std::vector<std::string> newDiscoverage {".gitignore",
                                             "hellovector.cpp",
                                             "Makefile"};
    std::cout << "New discoverage content: " << std::endl;
    printVector(newDiscoverage);

    std::vector<std::string> oldDiscoverage {data};
    std::cout << "Old discoverage content: " << std::endl;
    printVector(oldDiscoverage);
    std::cout << "After swapping, oldDiscoverage content: " << std::endl;

    newDiscoverage.swap(oldDiscoverage); // exchanges content
    printVector(oldDiscoverage);

    oldDiscoverage.erase(oldDiscoverage.begin()); // removes first element
    std::cout << "After erasing first element old discoverage is: " << std::endl;
    printVector(oldDiscoverage);

    std::cout << "Erasing all: " << std::endl;
    oldDiscoverage.erase(oldDiscoverage.begin(), oldDiscoverage.end()); // removes elements
    printVector(oldDiscoverage);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void investigateComparators()
{
    // Comparators
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::vector<unsigned int> defaultSettingsValues {0, 0, 1, 255, 255, 127};
    std::cout << "Default settings values: " << std::endl;
    printVector(defaultSettingsValues);

    std::vector<unsigned int> userSettingsValues {0, 0, 1, 255, 255, 127};
    std::cout << "User settings values: " << std::endl;
    printVector(userSettingsValues);

    std::cout << "User has default settings: " << (defaultSettingsValues == userSettingsValues ? "true" : "false") << std::endl;

    userSettingsValues.back() = 100;
    std::cout << "User has changed the settings! " << std::endl;

    std::cout << "User settings are less than default: " << (userSettingsValues < defaultSettingsValues ? "true" : "false") << std::endl;
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void investigateOperations()
{
    std::vector<std::string> fileList {
        "hello.h",
        "world.png",
        "we.jpg",
        "are.wav",
        "investigating.iso",
        "vector.cpp"
    };

    std::cout << "File list content: " << std::endl;
    printVector(fileList);

    investigateCapacityOperations(fileList);
    investigateAccessOperations(fileList);
    investigateModifiers(fileList);
    investigateComparators();
}

int main()
{
    investigateConstructors();
    investigateOperations();
    return 0;
}
