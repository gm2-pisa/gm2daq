#ifndef HCAL_AMC13_FILEPROMPT_HH_INCLUDED
#define HCAL_AMC13_FILEPROMPT_HH_INCLUDED 1

#include <vector>
#include <string>
#include <stdio.h>
#include <iostream>
#include <ctype.h>
#include <malloc.h>

#include <readline/readline.h>
#include <readline/history.h>

//
// simple class to handle readline-style input plus script files
//

class FilePrompt {
public:
  //Constructor and Destructor
  FilePrompt() {};
  ~FilePrompt() {};

  //Functions to handle user prompts
  std::string myprompt(const std::string& prompt, const std::string& defval);
  std::string myprompt(const std::string& prompt) { return myprompt(prompt,""); };

  //Other functions to handle files
  bool open( std::string filename);
  void close();
  int nFile();
  FILE *cfp();
  std::string cfn();

private:
  //Private variables
  std::vector<FILE *> fps;
  std::vector<std::string> fns;

};

#endif //HCAL_AMC13_FILEPROMPT_HH_INCLUDED
