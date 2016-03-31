#include "Producer.h"

Producer::Producer(const std::string& uri, FileData& filedata)
    : m_prefix(uri), m_filedata(filedata), m_debug(true)
{
  // check file exists and get file size
  std::ifstream is(m_filedata.m_local_filename, std::ifstream::binary | std::ifstream::ate);
  if (!is) {
    std::cerr << "File " << m_filedata.m_local_filename << " does not exist!" << std::endl;
    exit(1);
  }
  std::ifstream::pos_type filesize(is.tellg());
  is.close();
  
  if (m_debug)
    std::cerr << "File " << m_filedata.m_local_filename << " exists and has size " << filesize << std::endl;

  // store file as ndn data objects for fast retrieval
  populateStore();

  m_face.setInterestFilter(ndn::InterestFilter(m_prefix),
                           bind(&Producer::onInterest, this, _1, _2),
                           bind(&Producer::onRegisterSuccess, this, _1),
                           bind(&Producer::onRegisterFailure, this, _1, _2));
  
  if (m_debug)
    std::cerr << "Data published with name: " << m_prefix << std::endl;
}

void
Producer::populateStore()
{
  if (m_debug)
    std::cerr << "Populating store..." << std::endl;

  size_t maxSegmentSize = ndn::MAX_NDN_PACKET_SIZE >> 1; // half of max ndn packet size
  std::vector<uint8_t> buffer(maxSegmentSize);
  std::ifstream is(m_filedata.m_local_filename, std::ios::binary);
  while (is.good()) {
    is.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    const auto numCharsRead = is.gcount();
    if (numCharsRead > 0) {
      auto data = std::make_shared<ndn::Data>(ndn::Name(m_prefix).appendSegment(m_store.size()));
      data->setContent(&buffer[0], numCharsRead);
      m_store.push_back(data);
    }
  }

  if (m_store.empty()) {
    auto data = std::make_shared<ndn::Data>(ndn::Name(m_prefix).appendSegment(0));
    m_store.push_back(data);
  }
  auto finalBlockId = ndn::name::Component::fromSegment(m_store.size() - 1);
  for (const auto& data : m_store) {
    data->setFinalBlockId(finalBlockId);
    m_keyChain.sign(*data);
  }

  if (m_debug)
    std::cerr << "Store has size " << m_store.size() << std::endl;

  if (m_debug) {
    // TODO investigate why this debug file doesn't match the original file!
    std::cerr << "Writing debug file " << m_filedata.m_local_filename << ".debug" << std::endl;
    std::map<uint64_t, const char*> bufferedData;
    for (const auto& data : m_store) {
      const ndn::Block& block = data->getContent();
      bufferedData[data->getName()[-1].toSegment()] = reinterpret_cast<const char*>(block.value());
    }
    std::ofstream of(m_filedata.m_local_filename+".debug", std::ofstream::binary);
    if (!of) {
      std::cout << "ERROR: could not open " << m_filedata.m_local_filename << " for write" << std::endl;
      exit(1);
    }
    for (const auto &it : bufferedData) {
      of << it.second;
    }
    of.close();
  }
    
}

void
Producer::onInterest(const ndn::InterestFilter& filter, const ndn::Interest& interest)
{
  if (m_debug)
    std::cerr << "Interest: " << interest << std::endl;

#if 0
  std::cout << "Received Interest " << interest << std::endl;

  std::shared_ptr<ndn::Data> response(std::make_shared<ndn::Data>(interest.getName()));

  const std::string message = "Hello world, this is god speaking!";
  response->setContent(reinterpret_cast<const uint8_t*>(message.c_str()),
                       message.size());

  m_keyChain.sign(*response);
  m_face.put(*response);
#endif

#if 1
  // TODO verify this works
  const ndn::Name& name = interest.getName();
  std::shared_ptr<ndn::Data> data;
  
  // discovery interest or segment retrieval
  if ((name.size() == (m_prefix.size() + 1)) && m_prefix.isPrefixOf(name) && name[-1].isSegment()) {
    const auto segmentNum = static_cast<size_t>(name[-1].toSegment());
    if (segmentNum < m_store.size()) {
      // segment retrieval
      data = m_store[segmentNum];
    }
  } else if (interest.matchesData(*m_store[0])) {
    // discovery interest so look for first segment
    data = m_store[0];
  }

  if (data) {
    if (m_debug)
      std::cerr << "Data: " << *data << std::endl;

    m_face.put(*data);
  }
#endif
}

