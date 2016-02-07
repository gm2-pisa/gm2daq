#ifndef __STATUS_HH__
#define __STATUS_HH__

//AMC13 & uhal interface
#include "amc13/AMC13Simple.hh"

#include <ostream>
#include <iostream> //for std::cout
#include <vector>
#include <string>
#include <map>
#include <boost/unordered_map.hpp>
#include <boost/tokenizer.hpp> //for tokenizer

namespace amc13{

  const char ParameterParseToken = '_';
  const std::string DefaultFormat = "X";

  class Cell{
  public:
    Cell(){Clear();};
    void Setup(std::string const & _address,  //stripped of Hi/Lo
	       std::string const & _description,
	       std::string const & _row,
	       std::string const & _col,
	       std::string const & _format,
	       std::string const & _rule,
	       std::string const & _statusLevel);
    void Fill(uhal::ValWord<uint32_t> value,
	      size_t bitShift = 0);
    bool Display(int level,bool force = false);
    int DisplayLevel() {return statusLevel;};
    std::string Print(int width,bool html = false);
    std::string const & GetRow(){return row;};
    std::string const & GetCol(){return col;};
    std::string const & GetDesc(){return description;};
    std::string const & GetAddress(){return address;};
    void SetAddress(std::string const & _address){address = _address;};
    uint32_t const & GetMask(){return mask;};
    void SetMask(uint32_t const & _mask){mask = _mask;};

  private:    
    void Clear();
    void CheckAndThrow(std::string const & name,
		       std::string & thing1,
		       std::string const & thing2);
    uint64_t ComputeValue();

    std::string address;
    std::string description;
    std::string row;
    std::string col;

    uint32_t mask;
    
    std::vector<uhal::ValWord<uint32_t> > valWord;
    std::vector<int> valWordShift;
    std::string format;
    std::string displayRule;   
    int statusLevel;
  };

  class SparseCellMatrix{
  public:
    SparseCellMatrix(){Clear();};
    ~SparseCellMatrix(){Clear();};
    void Add(std::string chipName,uhal::Node const & node);
    void Render(std::ostream & stream,int status,bool HTML= false, bool LaTeX= false);
  private:
    void Clear();
    void CheckName(std::string const & newName);
    std::string ParseRow(boost::unordered_map<std::string,std::string> & parameters,
			 std::string const & addressBase);
    std::string ParseCol(boost::unordered_map<std::string,std::string> & parameters,
			 std::string const & addressBase);

    std::vector<Cell*> row(std::string const &);
    std::vector<Cell*> col(std::string const &);


    void Print(std::ostream & stream,int status,bool force,int headerColWidth,
	       std::map<std::string,bool> & rowDisp,std::vector<int> & colWidth);
    void PrintHTML(std::ostream & stream,int status,bool force,int headerColWidth,
		   std::map<std::string,bool> & rowDisp,std::vector<int> & colWidth);
    void PrintLaTeX(std::ostream & stream);

    std::string name;
    std::map<std::string,Cell*> cell;
    std::map<std::string,std::map<std::string,Cell *> > rowColMap;
    std::map<std::string,std::map<std::string,Cell *> > colRowMap;    
    std::vector<std::string> rowName;
    std::vector<std::string> colName;
  };
  
  class Status{
  public:
    Status(AMC13Simple * _amc13,int _version);
    void Report(size_t level,
		std::ostream & stream=std::cout,
		std::string const & singleTable = std::string(""));
    void SetHTML(){HTML=true;};
    void UnsetHTML(){HTML=false;};
    void SetLaTeX() {LaTeX=true;};
    void UnsetLaTeX() {LaTeX=false;};
  private:
    void ProcessChip(AMC13Simple::Board chip,std::string const & singleTable);
    void SetAMCMask(uint32_t mask);
    void SetSFPMask(uint32_t mask);
    bool HTML;
    bool LaTeX;
    int version;
    AMC13Simple * amc13;
    std::map<std::string,SparseCellMatrix> tables;
  };
}
#endif
