/* 
 * File:   main.cpp
 * Author: root
 *
 * Created on 2016年11月24日, 上午11:33
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>

#include "addressbook.pb.h"
#include "order.pb.h"

using namespace std;

/*
 * 
 */
// This function fills in a Person message based on user input.
void PromptForAddress(tutorial::Person* person) {
  cout << "Enter person ID number: ";
  int id;
  cin >> id;
  person->set_id(id);
  cin.ignore(256, '\n');

  cout << "Enter name: ";
  getline(cin, *person->mutable_name());

  cout << "Enter email address (blank for none): ";
  string email;
  getline(cin, email);
  if (!email.empty()) {
    person->set_email(email);
  }

  while (true) {
    cout << "Enter a phone number (or leave blank to finish): ";
    string number;
    getline(cin, number);
    if (number.empty()) {
      break;
    }

    tutorial::Person::PhoneNumber* phone_number = person->add_phones();
    phone_number->set_number(number);

    cout << "Is this a mobile, home, or work phone? ";
    string type;
    getline(cin, type);
    if (type == "mobile") {
      phone_number->set_type(tutorial::Person::MOBILE);
    } else if (type == "home") {
      phone_number->set_type(tutorial::Person::HOME);
    } else if (type == "work") {
      phone_number->set_type(tutorial::Person::WORK);
    } else {
      cout << "Unknown phone type.  Using default." << endl;
    }
  }
}

//// Main function:  Reads the entire address book from a file,
////   adds one person based on user input, then writes it back out to the same
////   file.
//int main(int argc, char* argv[]) {
//  // Verify that the version of the library that we linked against is
//  // compatible with the version of the headers we compiled against.
//  GOOGLE_PROTOBUF_VERIFY_VERSION;
//
//  if (argc != 2) {
//    cerr << "Usage:  " << argv[0] << " ADDRESS_BOOK_FILE" << endl;
//    return -1;
//  }
//
//  tutorial::AddressBook address_book;
//
//  {
//    // Read the existing address book.
//    fstream input(argv[1], ios::in | ios::binary);
//    if (!input) {
//      cout << argv[1] << ": File not found.  Creating a new file." << endl;
//    } else if (!address_book.ParseFromIstream(&input)) {
//      cerr << "Failed to parse address book." << endl;
//      return -1;
//    }
//  }
//
//  // Add an address.
//  PromptForAddress(address_book.add_people());
//
//  {
//    // Write the new address book back to disk.
//    fstream output(argv[1], ios::out | ios::trunc | ios::binary);
//    if (!address_book.SerializeToOstream(&output)) {
//      cerr << "Failed to write address book." << endl;
//      return -1;
//    }
//  }
//
//  // Optional:  Delete all global objects allocated by libprotobuf.
//  google::protobuf::ShutdownProtobufLibrary();
//
//  return 0;
//}

void initOrder(tutorial::order* order) {
  order->set_action(100);
  order->set_serialno("100abc");       
  order->set_version("1.00.003");
  string code = "TF0001";
  order->add_code(code);
  order->add_code(code);
  //order->set_code(code);
  string name = "test";
  order->set_name(name);
  order->set_price("10.01");
  order->set_amount("10000000.00");
}

int main(int argc, char* argv[])
{
         GOOGLE_PROTOBUF_VERIFY_VERSION;

         // 组装报文
         tutorial::order order;
         initOrder(&order);

         // 对象序列化为string
         string order_str;
         order.SerializeToString(&order_str);

         cout << order_str << endl;
         // 显示调式报文
         string order_debug = order.DebugString(); 
         cout << order_debug << endl;

         // string反序列化为对象
         tutorial::order  order_2;
         order_2.ParseFromString(order_str);
         
         for (int i = 0; i < order_2.code_size(); ++i)
         {
            cout << order_2.code(i) << endl;
         }
         
         
         
         cout << order_2.name() << endl;

         google::protobuf::ShutdownProtobufLibrary();

         getchar();

         return 0;
}