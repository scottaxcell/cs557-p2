#ifndef PRODUCER_H
#define PRODUCER_H

#include <iostream>
#include <fstream>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>

#include "fshare.h"

class Producer
{
public:
  Producer(const std::string& uri, FileData& filedata);

  void
  start()
  {
    m_face.processEvents();
  }

  void
  onRegisterSuccess(const ndn::Name& prefix)
  {
    std::cout << "Successfully registered " << prefix << std::endl;
  }

  void
  onRegisterFailure(const ndn::Name& prefix, const std::string& reason)
  {
    std::cout << "Failed to register prefix " << prefix << ": " << reason << std::endl;
  }

  void
  populateStore();

protected:

  void
  onInterest(const ndn::InterestFilter& filter, const ndn::Interest& interest);


protected:
  const ndn::Name m_prefix;
  ndn::Face m_face;
  ndn::KeyChain m_keyChain;
  FileData m_filedata;
  std::vector<std::shared_ptr<ndn::Data>> m_store;
  bool m_debug;
};

#endif
