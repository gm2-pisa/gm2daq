

#include "hcal/amc13/FilePrompt.hh"


// get a string from standard input or from active script file
//
std::string FilePrompt::myprompt(const std::string& prompt, const std::string& defval) {

  std::string retval;
  std::string trueprompt(prompt);

  if( cfp()) {
    // get input from file
    char buff[256];
    if( fgets( buff, 255, cfp() ) == NULL) {   // end of file or error, same diff
      close();
      return myprompt( prompt, defval);	// recurse
    } else {
      // clean trailing stuff from buff
      if( strlen( buff)) {
	for( int i=strlen(buff)-1; i && iscntrl(buff[i]); i--)
	  buff[i] = '\0';
      }
      retval = buff;
    }

  } else {
    // get input from stdin using readline

    char* res=readline(trueprompt.c_str());
    if( res == NULL) {
      std::cerr << "EOF on command input" << std::endl;
      throw 0;
    }
    retval=std::string(res);
    free(res);
    if (retval.empty()) retval=defval;
    // path autocompletion when tabulation hit
    //rl_bind_key('\t', rl_complete);
    // adding the previous input into history
    add_history(retval.c_str());
  }

  return retval;
}

// see if there is a file open
int FilePrompt::nFile() { return fps.size(); }

// return current fp
FILE *FilePrompt::cfp() { return nFile() ? fps[nFile()-1] : NULL; }

// return current file name
std::string FilePrompt::cfn() { return nFile() ? fns[nFile()-1] : ""; }

// close file
void FilePrompt::close() {
  if( cfp())
    fclose( cfp());
  if( nFile()) {
    fps.pop_back();
    fns.pop_back();
  }
}

// open a script file
bool FilePrompt::open( std::string f) {
  FILE *fp;
  if( (fp = fopen( f.c_str(), "r")) == NULL) {
    return false;
  } else {
    fns.push_back( f);
    fps.push_back( fp);
  }
  return true;
}
