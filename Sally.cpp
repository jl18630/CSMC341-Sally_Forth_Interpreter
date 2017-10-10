// Kyle Lee                                                                     
//                                                                              
// File: Sally.cpp                                                              
//                                                                              
// CMSC 341 Spring 2017 Project 2                                               
//                                                                              
// Implementation of member functions of Sally Forth interpreter                
//                                                                              

#include <iostream>
#include <string>
#include <list>
#include <stack>
#include <stdexcept>
#include <cstdlib>
using namespace std ;

#include "Sally.h"


// Basic Token constructor. Just assigns values.                                
//                                                                              
Token::Token(TokenKind kind, int val, string txt) {
   m_kind = kind ;
   m_value = val ;
   m_text = txt ;
}

// Basic SymTabEntry constructor. Just assigns values.                          
//                                                                              
SymTabEntry::SymTabEntry(TokenKind kind, int val, operation_t fptr) {
   m_kind = kind ;
   m_value = val ;
   m_dothis = fptr ;
}


// Constructor for Sally Forth interpreter.                                     
// Adds built-in functions to the symbol table.                                 
//                                                                              
Sally::Sally(istream& input_stream) :
   istrm(input_stream)  // use member initializer to bind reference             
{
   symtab["DUMP"]    =  SymTabEntry(KEYWORD,0,&doDUMP) ;

   symtab["+"]    =  SymTabEntry(KEYWORD,0,&doPlus) ;
   symtab["-"]    =  SymTabEntry(KEYWORD,0,&doMinus) ;
   symtab["*"]    =  SymTabEntry(KEYWORD,0,&doTimes) ;
   symtab["/"]    =  SymTabEntry(KEYWORD,0,&doDivide) ;
   symtab["%"]    =  SymTabEntry(KEYWORD,0,&doMod) ;
   symtab["NEG"]  =  SymTabEntry(KEYWORD,0,&doNEG) ;

   symtab["."]    =  SymTabEntry(KEYWORD,0,&doDot) ;
   symtab["SP"]   =  SymTabEntry(KEYWORD,0,&doSP) ;
   symtab["CR"]   =  SymTabEntry(KEYWORD,0,&doCR) ;

   symtab["<"] = SymTabEntry(KEYWORD,0,&doLessThan);
   symtab["<="] = SymTabEntry(KEYWORD,0,&doLessOrEqual);
   symtab[">"] = SymTabEntry(KEYWORD,0,&doGreaterThan);
   symtab[">="] = SymTabEntry(KEYWORD,0,&doGreaterOrEqual);
   symtab["=="] = SymTabEntry(KEYWORD,0,&doEqualEqual);
   symtab["!="] = SymTabEntry(KEYWORD,0,&doNotEqual);
       
   symtab["DUP"] = SymTabEntry(KEYWORD,0,&doDUP);
   symtab["SWAP"] = SymTabEntry(KEYWORD,0,&doSWAP);
   symtab["DROP"] = SymTabEntry(KEYWORD,0,&doDROP);
   symtab["ROT"] = SymTabEntry(KEYWORD,0,&doROT);

   symtab["SET"] = SymTabEntry(KEYWORD,0,&doSET);
   symtab["@"] = SymTabEntry(KEYWORD,0,&doAT);
   symtab["!"] = SymTabEntry(KEYWORD,0,&doSTORE);

   symtab["AND"] = SymTabEntry(KEYWORD,0,&doAND);
   symtab["OR"] = SymTabEntry(KEYWORD,0,&doOR);
   symtab["NOT"] = SymTabEntry(KEYWORD,0,&doNOT);

   symtab["IFTHEN"] = SymTabEntry(KEYWORD,0,&doIFTHEN);
   symtab["ENDIF"] = SymTabEntry(KEYWORD,0,&doENDIF);
   symtab["ELSE"] = SymTabEntry(KEYWORD,0,&doELSE);

   symtab["DO"] = SymTabEntry(KEYWORD,0,&doDO);
   symtab["UNTIL"] = SymTabEntry(KEYWORD,0,&doUNTIL);
}


// This function should be called when tkBuffer is empty.                       
// It adds tokens to tkBuffer.                                                  
//                                                                              
// This function returns when an empty line was entered                         
// or if the end-of-file has been reached.                                      
//                                                                              
// This function returns false when the end-of-file was encountered.            
//                                                                              
// Processing done by fillBuffer()                                              
//   - detects and ignores comments.                                            
//   - detects string literals and combines as 1 token                          
//   - detetcs base 10 numbers                                                  
//                                                                              
//  
bool Sally::fillBuffer() {
   string line ;     // single line of input                                    
   int pos ;         // current position in the line                            
   int len ;         // # of char in current token                              
   long int n ;      // int value of token                                      
   char *endPtr ;    // used with strtol()                                      


   while(true) {    // keep reading until empty line read or eof                

      // get one line from standard in                                          
      //                                                                        
      getline(istrm, line) ;

      // if "normal" empty line encountered, return to mainLoop                 
      //                                                                        
      if ( line.empty() && !istrm.eof() ) {
         return true ;
      }

      // if eof encountered, return to mainLoop, but say no more                
      // input available                                                        
      //                                                                        
      if ( istrm.eof() )  {
     return false ;
      }


      // Process line read                                                      

      pos = 0 ;                      // start from the beginning                

      // skip over initial spaces & tabs                                        
      //                                                                        
      while( line[pos] != '\0' && (line[pos] == ' ' || line[pos] == '\t') ) {
        pos++ ;
      }

      // Keep going until end of line                                           
      //                                                                        
      while (line[pos] != '\0') {

         // is it a comment?? skip rest of line.                                
     //                                                                         
         if (line[pos] == '/' && line[pos+1] == '/') break ;

         // is it a string literal?                                             
     //                                                                         
         if (line[pos] == '.' && line[pos+1] == '"') {

        pos += 2 ;  // skip over the ."                                         
        len = 0 ;   // track length of literal                                  

            // look for matching quote or end of line                           
        //                                                                      
        while(line[pos+len] != '\0' && line[pos+len] != '"') {
           len++ ;
        }

            // make new string with characters from                             
        // line[pos] to line[pos+len-1]                                         
        string literal(line,pos,len) ;  // copy from pos for len chars          

            // Add to token list                                                
        //                                                                      
        tkBuffer.push_back( Token(STRING,0,literal) ) ;

            // Different update if end reached or " found                       
            //                                                                  
        if (line[pos+len] == '\0') {
           pos = pos + len ;
        } else {
           pos = pos + len + 1 ;
        }

     } else {  // otherwise "normal" token                                      

        len = 0 ;  // track length of token  

        // line[pos] should be an non-white space character                 
        // look for end of line or space or tab                                 
	    //                                                                      
        while(line[pos+len] != '\0' && line[pos+len] != ' ' && line[pos+len] !=\'\t') {
           len++ ;
        }

        string literal(line,pos,len) ;   // copy form pos for len chars         
        pos = pos + len ;

            // Try to convert to a number                                       
        //                                                                      
            n = strtol(literal.c_str(), &endPtr, 10) ;

            if (*endPtr == '\0') {
           tkBuffer.push_back( Token(INTEGER,n,literal) ) ;
        } else {
           tkBuffer.push_back( Token(UNKNOWN,0,literal) ) ;
        }
     }
          
     // skip over trailing spaces & tabs                                        
     //                                                                         
     while( line[pos] != '\0' && (line[pos] == ' ' || line[pos] == '\t') ) {
        pos++ ;
     }

      }
   }
}

// Return next token from tkBuffer.                                             
// Call fillBuffer() if needed.                                                 
// Checks for end-of-file and throws exception                                  
//                                                                              
Token Sally::nextToken() {
      Token tk ;
      bool more = true ;

      while(more && tkBuffer.empty() ) {
         more = fillBuffer() ;
      }

      if ( !more && tkBuffer.empty() ) {
         throw EOProgram("End of Program") ;
      }

      tk = tkBuffer.front() ;
      tkBuffer.pop_front() ;
      return tk ;
}

// The main interpreter loop of the Sally Forth interpreter.                    
// It gets a token and either push the token onto the parameter                 
// stack or looks for it in the symbol table.                                   
//                                                                              
//                                                                              
void Sally::mainLoop() {

   Token tk ;
   map<string,SymTabEntry>::iterator it ;

   try {
      while( 1 ) {
     tk = nextToken() ;

     if (tk.m_kind == INTEGER || tk.m_kind == STRING) {

            // if INTEGER or STRING just push onto stack                        
        params.push(tk) ;

     } else {
        it = symtab.find(tk.m_text) ;

        if ( it == symtab.end() )  {   // not in symtab                         

           params.push(tk) ;

        } else if (it->second.m_kind == KEYWORD)  {

           // invoke the function for this operation                            
           //                                                                   
           it->second.m_dothis(this) ;
        } else if (it->second.m_kind == VARIABLE) {

               // variables are pushed as tokens                                
           //                                                                   
           tk.m_kind = VARIABLE ;
           params.push(tk) ;

        } else {

               // default action                                                
           //                                                                   
           params.push(tk) ;

        }
     }
      }
       
   } catch (EOProgram& e) {

      cerr << "End of Program\n" ;
      if ( params.size() == 0 ) {
         cerr << "Parameter stack empty.\n" ;
      } else {
         cerr << "Parameter stack has " << params.size() << " token(s).\n" ;
      }

   } catch (out_of_range& e) {
     cerr << e.what() << endl;
      cerr << "Parameter stack underflow??\n" ;

   } catch (...) {

      cerr << "Unexpected exception caught\n" ;

   }
}


// -------------------------------------------------------                      

// Name: doPlus                                                                 
// Input: Sptr                                                                  
// Output: pushes sum by addition into stack                                    
void Sally::doPlus(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for +.") ;
   }

   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   //returns sum                                                                
   int answer = p2.m_value + p1.m_value ;
   //push sum into parameter stack                                              
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

// Name: doMinus                                                                
// Input: Sptr                                                                  
// Output: pushes sum by subtraction into stack                                 
void Sally::doMinus(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for -.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value - p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

// Name: doTimes                                                                
// Input: Sptr                                                                  
// Output: pushes product into stack                                            
void Sally::doTimes(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for *.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value * p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

// Name: doDivide                                                               
// Input: Sptr                                                                  
// Output: pushes quotient into stack                                           
void Sally::doDivide(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for /.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value / p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

// Name: doMod                                                                  
// Output: Sptr                                                                 
// Input: pushes remainder into stack                                           
void Sally::doMod(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for %.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value % p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

// Name: doNEG                                                                  
// Input: Sptr                                                                  
// Output: pushes the opposite boolean value into stack                         
void Sally::doNEG(Sally *Sptr) {
   Token p1 ;

   if ( Sptr->params.size() < 1 ) {
      throw out_of_range("Need one parameter for NEG.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;

   //pushes the negative of the boolean value into parameter stack              
   Sptr->params.push( Token(INTEGER, -p1.m_value, "") ) ;
}

void Sally::doDot(Sally *Sptr) {

   Token p ;
   if ( Sptr->params.size() < 1 ) {
      throw out_of_range("Need one parameter for Dot.") ;
   }

   p = Sptr->params.top() ;
   Sptr->params.pop() ;

   if (p.m_kind == INTEGER) {
      cout << p.m_value ;
   } else {
      cout << p.m_text ;
   }
}

void Sally::doSP(Sally *Sptr) {
   cout << " " ;
}


void Sally::doCR(Sally *Sptr) {
   cout << endl ;
}

void Sally::doDUMP(Sally *Sptr) {
   // do whatever for debugging                                                 
}

// Name: doLessThan                                                             
// Input: Sptr                                                                  
// Output: pushes 1 or 0 through comparison into stack                          
void Sally::doLessThan(Sally *Sptr){
  Token p1, p2 ;

  if ( Sptr->params.size() < 2 ) {
    throw out_of_range("Need two parameters for doLessThan.") ;
  }
  p1 = Sptr->params.top() ;
  Sptr->params.pop() ;
  p2 = Sptr->params.top() ;
  Sptr->params.pop() ;

  //returns 1 if second token is less than first token, 0 otherwise             
  int answer = p2.m_value < p1.m_value;
  Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


// Name: doLessOrEqual                                                          
// Input: Sptr                                                                  
// Output: returns 1 or 0 into stack                                            
void Sally::doLessOrEqual(Sally *Sptr){
  Token p1, p2 ;

  if ( Sptr->params.size() < 2 ) {
    throw out_of_range("Need two parameters for doLessOrEqual.") ;
  }
  p1 = Sptr->params.top() ;
  Sptr->params.pop() ;
  p2 = Sptr->params.top() ;
  Sptr->params.pop() ;
  int answer = p2.m_value <= p1.m_value ;
  Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


// Name: doGreaterThan                                                          
// Input: Sptr                                                                  
// Output: pushes 1 or 0 through comparison into stack                          
void Sally::doGreaterThan(Sally *Sptr){
  Token p1, p2 ;

  if ( Sptr->params.size() < 2 ) {
    throw out_of_range("Need two parameters for doGreaterThan.") ;
  }
  p1 = Sptr->params.top() ;
  Sptr->params.pop() ;
  p2 = Sptr->params.top() ;
  Sptr->params.pop() ;
  int answer = p2.m_value > p1.m_value ;
  Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


// Name: doGreaterOrEqual                                                       
// Input: Sptr                                                                  
// Output: returns 1 or 0 through comparison into stack                         
void Sally::doGreaterOrEqual(Sally *Sptr){
  Token p1, p2 ;

  if ( Sptr->params.size() < 2 ) {
    throw out_of_range("Need two parameters for doGreaterOrEqual.") ;
  }
  p1 = Sptr->params.top() ;
  Sptr->params.pop() ;
  p2 = Sptr->params.top() ;
  Sptr->params.pop() ;
  int answer = p2.m_value >= p1.m_value ;
  Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

// Name: doEqualEqual                                                           
// Input: Sptr                                                                  
// Output: return 1 or 0 through comparison into stack                          
void Sally::doEqualEqual(Sally *Sptr){
  Token p1, p2 ;

  if ( Sptr->params.size() < 2 ) {
    throw out_of_range("Need two parameters for doEqualEqual.") ;
  }
  p1 = Sptr->params.top() ;
  Sptr->params.pop() ;
  p2 = Sptr->params.top() ;
  Sptr->params.pop() ;
  int answer = p2.m_value == p1.m_value ;
  Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

// Name: doNotEqual                                                             
// Input: Sptr                                                                  
// Output: push 1 or 0 through comparison into stack                            
void Sally::doNotEqual(Sally *Sptr){
  Token p1, p2 ;

  if ( Sptr->params.size() < 2 ) {
    throw out_of_range("Need two parameters for doNotEqual.") ;
  }
  p1 = Sptr->params.top() ;
  Sptr->params.pop() ;
  p2 = Sptr->params.top() ;
  Sptr->params.pop() ;

  int answer = p2.m_value != p1.m_value ;

  Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

// Name: doDUP                                                                  
// Input: Sptr                                                                  
// Output: push duplicate values into stack                                     
void Sally::doDUP(Sally *Sptr){
  Token p1, p2;

  if ( Sptr->params.size() < 1 ) {
    throw out_of_range("Need two parameters for DUP.") ;
  }
  p1 = Sptr -> params.top();
  p2 = Sptr -> params.top();

  int duplicate1 = p1.m_value;
  int duplicate2 = p2.m_value;

  Sptr -> params.pop();

  Sptr -> params.push(Token(INTEGER, duplicate1, ""));
  Sptr -> params.push(Token(INTEGER, duplicate2, ""));
}

// Name: doDROP                                                                 
// Input: Sptr                                                                  
// Output: pop off top of stack                                                 
void Sally::doDROP(Sally *Sptr){
  if ( Sptr->params.size() < 1 ) {
    throw out_of_range("Need one parameter for DROP.") ;
  }

  Sptr -> params.pop();
}


// Name: doSWAP                                                                 
// Input: Sptr                                                                  
// Output: swap values of two tokens and push them back into stack              
void Sally::doSWAP(Sally *Sptr){
  Token p1, p2;

  if ( Sptr->params.size() < 2 ) {
    throw out_of_range("Need two parameters for SWAP.") ;
  }

  p1 = Sptr -> params.top();
  Sptr -> params.pop();
  p2 = Sptr -> params.top();
  Sptr -> params.pop();

  int num1 = p2.m_value;
  int num2 = p1.m_value;

  Sptr -> params.push(Token(INTEGER, num1, ""));
  Sptr -> params.push(Token(INTEGER, num2, ""));
}


// Name: doROT                                                                  
// Input: Sptr                                                                  
// Output: rotate tokens                                                        
void Sally::doROT(Sally *Sptr){
  Token p1, p2, p3;

  if ( Sptr->params.size() < 3 ) {
    throw out_of_range("Need three parameters for ROT.") ;
  }

  p1 = Sptr -> params.top();
  Sptr -> params.pop();
  p2 = Sptr -> params.top();
  Sptr -> params.pop();
  p3 = Sptr -> params.top();
  Sptr -> params.pop();

  int top = p1.m_value;
  int middle = p2.m_value;
  int bottom = p3.m_value;

  Sptr -> params.push(Token(INTEGER, middle, ""));
  Sptr -> params.push(Token(INTEGER, top, ""));
  Sptr -> params.push(Token(INTEGER, bottom, ""));
}

// Name: doSET                                                                  
// Input: Sptr                                                                  
// Output: sets a new keyword with a value                                      
void Sally::doSET(Sally *Sptr){
  Token p1, p2;

  if ( Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for SET.") ;
  }

  p1 = Sptr -> params.top();
  Sptr -> params.pop();
  p2 = Sptr -> params.top();
  Sptr -> params.pop();

  //If there is an existing keyword, print error and carry on                   
  if(Sptr -> symtab.count(p1.m_text)){
    cout << "ERROR SET" << endl;
  }
  //if there is no already existing keyword...                                  
  else{
    //Create new symtab with new symbol                                         
    Sptr -> symtab[p1.m_text] = SymTabEntry(VARIABLE, p2.m_value);
  }
}


// Name: doAt                                                                   
// Input: Sptr                                                                  
// Output: retrieve the value of a keyword from the symtab                      
void Sally::doAT(Sally *Sptr){
  Token p1;
  int value = 0;

  if ( Sptr->params.size() < 1){
    throw out_of_range("Need one parameter for AT.") ;
  }

  p1 = Sptr -> params.top();
  Sptr -> params.pop();

  //check to see if existing keyword already exists in symtab                   
  if(Sptr -> symtab.count(p1.m_text)){
    //set the value of symtab as 'value' variable                               
    value = Sptr -> symtab.find(p1.m_text) -> second.m_value;
    Sptr -> params.push(Token(INTEGER, value));
  }
  else{
    cout << "ERROR @" << endl;
  }
}


// Name: doSTORE                                                                
// Input: Sptr                                                                  
// Output: stores the value for the keyword using the '!' character             
void Sally::doSTORE(Sally *Sptr){
  Token p1, p2;

  if(Sptr -> params.size() < 1){
    throw out_of_range("Need one parameter for STORE");
  }
  p1 = Sptr -> params.top();
  Sptr -> params.pop();
  p2 = Sptr -> params.top();
  Sptr -> params.pop();

  if(Sptr -> symtab.count(p1.m_text)){
    Sptr -> symtab[p1.m_text] = SymTabEntry(VARIABLE, p2.m_value);
  }
  else{
    cout << "ERROR STORE" << endl;
  }
}


// Name: doAND                                                                  
// Input: Sptr                                                                  
// Output: push 1 or 0 for AND into the stack                                   
void Sally::doAND(Sally *Sptr){
  Token p1, p2 ;

  if ( Sptr->params.size() < 2 ) {
    throw out_of_range("Need two parameters for AND.") ;
  }
  p1 = Sptr->params.top() ;
  Sptr->params.pop() ;
  p2 = Sptr->params.top() ;
  Sptr->params.pop() ;

  int answer = p1.m_value && p2.m_value ;
  Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


// Name: doOR                                                                   
// Input: Sptr                                                                  
// Output: push 1 or 0 for OR into the stack                                    
void Sally::doOR(Sally *Sptr){
  Token p1, p2 ;

  if ( Sptr->params.size() < 2 ) {
    throw out_of_range("Need two parameters for AND.") ;
  }
  p1 = Sptr->params.top() ;
  Sptr->params.pop() ;
  p2 = Sptr->params.top() ;
  Sptr->params.pop() ;

  int answer = p1.m_value || p2.m_value ;
  Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


// Name: doNOT                                                                  
// Input: Sptr                                                                  
// Output: pushes the negation of the boolean value into the stack              
void Sally::doNOT(Sally *Sptr){
  Token p1;
  int answer = 1;

  if ( Sptr->params.size() < 1 ) {
    throw out_of_range("Need one parameter for NOT.") ;
  }
  p1 = Sptr->params.top() ;
  Sptr->params.pop() ;

  //if true...                                                                  
  if(p1.m_value == 1){
    //push false                                                                
    answer = 0;
    Sptr->params.push( Token(INTEGER, answer, "") ) ;
  }
  //if false                                                                    
  else{
    //push true                                                                 
    Sptr->params.push( Token(INTEGER, answer, "") ) ;
  }
}


// Name: doIFTHEN                                                               
// Input: Sptr                                                                  
// Output: skip over conditions nested in IFTHEN if IFTHEN condition is false   
void Sally::doIFTHEN(Sally *Sptr){
  Token p1;

  if ( Sptr->params.size() < 1 ) {
    throw out_of_range("Need 1 parameter for IFTHEN.") ;
  }

  p1 = Sptr -> params.top();
  Sptr -> params.pop();

  int counter = 1;

  //if outer condition is false...                                              
  if(p1.m_value == 0){
    //skip over nested conditions                                               
    while(counter > 0){
      p1 = Sptr -> nextToken();
      if(p1.m_text == "IFTHEN")
        counter++;
      if(p1.m_text == "ELSE")
        counter--;
    }
  }
}


// Name: doElse                                                                 
// Input: Sptr                                                                  
// Output: skip over conditions nested in Else if Else condition is false       
void Sally::doELSE(Sally *Sptr){
  Token p1;

  int counter = 1;

  if(p1.m_value == 0){
    while(counter > 0){
      p1 = Sptr -> nextToken();
      if(p1.m_text == "ELSE")
        counter++;
      if(p1.m_text == "ENDIF")
        counter--;
    }
  }
}


// Name: doENDIF                                                                
// Input: Sptr                                                                  
// Output: terminate IFTHEN ELSE by return                                      
void Sally::doENDIF(Sally *Sptr){
  return;
}


// Name: doDO                                                                   
// Input: Sptr                                                                  
// Output: copy tokens into temporary list                                      
void Sally::doDO(Sally *Sptr){
  Token p1;

  p1 = Sptr -> nextToken();

  // While reaching 'UNTIL'                                                     
  while(p1.m_text != "UNTIL"){
    //push tokens into temporary list                                           
    Sptr -> temp1.push_back(p1);
    //traverse to the next token                                                
    p1 = Sptr -> nextToken();
  }

  //push 'UNTIL' into the temporary list                                        
  Sptr -> temp1.push_back(p1);
  //Set the temporary list as the tkBuffer to run                               
  Sptr -> tkBuffer = Sptr -> temp1;
}


// Name: doUNTIL                                                                
// Input: Sptr                                                                  
// Output: call the DO function until the condition is met                      
void Sally::doUNTIL(Sally *Sptr){
  Token p1;

  p1 = Sptr -> params.top();
  Sptr -> params.pop();

  if(p1.m_value == 0){
    while(p1.m_text != "UNTIL"){
      //iterate through the temporary list                                      
      for(list<Token>::iterator it = Sptr -> temp1.begin(); it != Sptr -> temp1\
.end(); it++){
        p1 = *it;
        Sptr -> tkBuffer.push_back(p1);
      }
    }
  }
}

