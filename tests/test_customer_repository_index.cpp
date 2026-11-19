#include <cassert>
#include <cstddef>
#include <map>
#include <string>

#include "Account/Customer.h"
#include "Repository/CustomerRepository.h"
#include "Storage/FileManager.h"

class MemoryFileManager : public FileManager {
public:
    mutable std::map<std::string, std::size_t> readCount;
    mutable std::map<std::string, LinkedList<std::string>> files;

    bool readAllLines(const std::string& path, LinkedList<std::string>& out,
                      std::string& error) const override {
        ++readCount[path];
        const auto file = files.find(path);
        if (file == files.end()) {
            error = "Missing test file: " + path;
            return false;
        }
        out = file->second;
        error.clear();
        return true;
    }

    bool writeAllLines(const std::string& path,
                       const LinkedList<std::string>& lines,
                       std::string& error) const override {
        files[path] = lines;
        error.clear();
        return true;
    }
};

int main() {
    MemoryFileManager fileManager;
    fileManager.files["data/customers.txt"].push_back(
        "C1|user1|pass|Customer One|0900000001|2|1");
    fileManager.files["data/accounts.txt"].push_back(
        "CUSTOMER|C1|user1|pass|Legacy Account Name|0900000088|2|1");

    CustomerRepository repository(fileManager);
    Customer customer;
    std::string error;
    assert(repository.findCustomerByUserId("C1", customer, error));
    assert(customer.get_name() == "Customer One");
    assert(repository.findCustomerByUserId("0900000001", customer, error));
    assert(repository.findCustomerByUserId("0900000088", customer, error));
    assert(customer.get_name() == "Legacy Account Name");
    assert(fileManager.readCount["data/customers.txt"] == 1);
    assert(fileManager.readCount["data/accounts.txt"] == 1);

    assert(repository.findCustomerByUserId("C1", customer, error));
    customer.update_phone_number("0900000099");
    assert(repository.updateCustomer(customer, error));
    assert(repository.findCustomerByUserId("0900000099", customer, error));
    assert(!repository.findCustomerByUserId("0900000001", customer, error));
    assert(!repository.findCustomerByUserId("0900000088", customer, error));
    assert(fileManager.readCount["data/accounts.txt"] == 2);

    Customer added("C2", "user2", "pass", "Customer Two", "0900000002");
    assert(repository.saveCustomer(added, error));
    const std::size_t readsAfterSave = fileManager.readCount["data/customers.txt"];
    assert(repository.findCustomerByUserId("0900000002", customer, error));
    assert(customer.getUserId() == "C2");
    assert(fileManager.readCount["data/customers.txt"] == readsAfterSave);
    assert(fileManager.readCount["data/accounts.txt"] == 2);

    fileManager.files["data/customers.txt"].clear();
    fileManager.files["data/customers.txt"].push_back(
        "C1|user1|pass|Customer One Reloaded|0900000099|2|1");
    fileManager.files["data/customers.txt"].push_back(
        "C2|user2|pass|Customer Two|0900000002|0|0");
    LinkedList<Customer> reloadedCustomers;
    assert(repository.loadAllCustomers(reloadedCustomers, error));
    assert(repository.findCustomerByUserId("C1", customer, error));
    assert(customer.get_name() == "Customer One Reloaded");
    assert(fileManager.readCount["data/customers.txt"] == readsAfterSave + 1);
    assert(fileManager.readCount["data/accounts.txt"] == 3);

    CustomerRepository reloadedRepository(fileManager);
    assert(reloadedRepository.findCustomerByUserId("C2", customer, error));
    assert(customer.get_name() == "Customer Two");
    assert(fileManager.readCount["data/customers.txt"] == readsAfterSave + 2);
    assert(fileManager.readCount["data/accounts.txt"] == 4);
}
