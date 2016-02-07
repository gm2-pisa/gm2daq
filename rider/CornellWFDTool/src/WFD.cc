/** @file WFD.cc

    Class for Cornell WFDs. Defines IPbus read and write commands.

    Modeled after AMC13Simple class in the AMC13Tool code:
        amc13_v1_0_5/amc13/src/common/AMC13Simple.cc
    (Much of the code is copied directly from there)

    One difference is that the AMC13 code has an exception class;
    we have not written a WFD exception class yet, so are using some
    generic options from std::exception for now.

    @author Robin Bjorkquist
    @date 2015

 */

#include "WFD.hh"

WFD::WFD(uhal::HwInterface* hw) :
  m_wfd(hw)
{
}


// IPbus read, given name of node
uint32_t WFD::read(const std::string& node)
{
  uhal::ValWord<uint32_t> ret;
  ret = m_wfd->getNode(node).read();
  m_wfd->dispatch();
  return ret;
}


// IPbus read, given address of node
uint32_t WFD::read(uint32_t addr)
{
  uhal::ValWord<uint32_t> ret;
  ret = m_wfd->getClient().read(addr);
  m_wfd->getClient().dispatch();
  return ret;
}


// IPbus block read, given name of node
size_t WFD::read(const std::string& node, size_t nWords, uint32_t* buffer)
{
  //Make sure we have a valid buffer
  if ( buffer==NULL ) {
    throw std::invalid_argument("NULL Pointer");
  }
    
  uhal::ValVector<uint32_t> retVec ;
  std::vector<uint32_t> stdVec ;
  uint32_t offset = 0 ;
  int remainingWords = nWords ;
    
  // Check if register has incremental mode
  if ( ( m_wfd->getNode(node).getMode() == uhal::defs::INCREMENTAL ) && ( nWords < MAX_BLOCK_READ_SIZE ) )
    {
      // Using register block read
      retVec = m_wfd->getNode(node).readBlock(nWords);
      m_wfd->dispatch() ;
      std::copy( retVec.begin(), retVec.end(), std::back_inserter( stdVec ) ) ;
    } 
  else
    {
      // Using address block read (starting address taken from reg)
      uint32_t nodeAddr = m_wfd->getNode(node).getAddress();
      while ( remainingWords )
	{
	  uint32_t wordsToRead = remainingWords > MAX_BLOCK_READ_SIZE ? MAX_BLOCK_READ_SIZE : remainingWords ; 
	  retVec = m_wfd->getClient().readBlock( ( nodeAddr + offset ), wordsToRead, uhal::defs::INCREMENTAL );
	  m_wfd->getClient().dispatch();    
	  std::copy( retVec.begin(), retVec.end(), std::back_inserter( stdVec ) ) ;
	  remainingWords -= wordsToRead ;
	  offset += wordsToRead ;
	}
    }
    
  // Make sure stdVec is equal or smaller than the number of requested words
  if ( stdVec.size() > nWords) {
    throw std::range_error("Unexpected Range");
  }
    
  // Copy Block read (stdVec) to buffer array
  for ( size_t i=0; i < stdVec.size(); i++) {
    buffer[i] = stdVec[i] ;
  }
    
  return stdVec.size() ;
}


// IPbus block read, given address of node
size_t WFD::read(uint32_t addr, size_t nWords, uint32_t* buffer)
{
  //Make sure we have a valid buffer
  if ( buffer==NULL ) {
    throw std::invalid_argument("NULL Pointer");
  }
        
  uhal::ValVector<uint32_t> retVec ;
  std::vector<uint32_t> stdVec ;
  uint32_t offset = 0 ;
  int remainingWords = nWords ;
    
  //Do the read, pass on uhal exceptions
  while( remainingWords ) {
    uint32_t wordsToRead = remainingWords > MAX_BLOCK_READ_SIZE ? MAX_BLOCK_READ_SIZE : remainingWords ; 
    retVec = m_wfd->getClient().readBlock( ( addr+offset ), wordsToRead, uhal::defs::INCREMENTAL );
    m_wfd->getClient().dispatch() ;
    std::copy( retVec.begin(), retVec.end(), std::back_inserter( stdVec ) ) ;
    remainingWords -= wordsToRead ;
    offset += wordsToRead ;
  }
    
  // Make sure stdVec is equal or smaller than the number of requested words
  if ( stdVec.size() > nWords) {
    throw std::range_error("Unexpected Range");
  }
    
  // Copy Block read (stdVec) to buffer array
  for ( size_t i=0; i <  stdVec.size(); i++) {
    buffer[i] = stdVec[i] ;
  }

  return stdVec.size() ;
}

// IPbus write, given name of node
void WFD::write(const std::string& node, uint32_t value)
{
  m_wfd->getNode(node).write(value);
  m_wfd->dispatch();
}

// IPbus write, given address of node
void WFD::write(uint32_t addr, uint32_t value)
{
  m_wfd->getClient().write(addr,value);
  m_wfd->getClient().dispatch();
}

// IPbus block write, given name of node
void WFD::write(const std::string& node, size_t nWords, uint32_t* data)
{
  //Make sure we have a valid buffer
  if ( data==NULL ) {
    throw std::invalid_argument("NULL Pointer");
  }
        
  //Create writeVector and load it with our data
  std::vector<uint32_t> writeVec;
  writeVec.resize(nWords);
  memcpy(&(writeVec[0]),data,nWords*sizeof(uint32_t));
    
  m_wfd->getNode( node ).writeBlock( writeVec );
  m_wfd->dispatch();  
}

// IPbus block write, given address of node
void WFD::write(uint32_t addr, size_t nWords, uint32_t* data)
{
  //Make sure we have a valid buffer
  if ( data==NULL ) {
    throw std::invalid_argument("NULL Pointer");
  }
 
  //Create writeVector and load it with our data
  std::vector<uint32_t> writeVec;
  writeVec.resize(nWords);
  memcpy( &(writeVec[0]), data, nWords*sizeof(uint32_t) );
  m_wfd->getClient().writeBlock( addr, writeVec, uhal::defs::INCREMENTAL );
  m_wfd->dispatch();  
}


