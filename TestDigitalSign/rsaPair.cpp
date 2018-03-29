/* 
 * File:   rsaPair.cpp
 * Author: root
 * 
 * Created on 2017年1月13日, 下午12:05
 */

#include "rsaPair.h"

using namespace std ;
rsaPair::rsaPair()
{
}

rsaPair::rsaPair(const rsaPair& orig)
{
}



rsaPair::rsaPair( string pub_k_path , string pri_k_path , string psword)
{
   pub_key = NULL ;
   pri_key = NULL ;


   if ( pub_k_path.empty () || pri_k_path.empty() )
   {
    perror ("file stores key values empty") ;
        return ;
   }
   
   if ( psword.empty ())
   {
    perror ("password empty , use default") ;
    password = "inuyasha" ;
      return ; 
  }

   printf ("here ") ;
  
   pub_key_path = pub_k_path ;
   pri_key_path = pri_k_path ;
    
   password = psword ;

}

rsaPair::~rsaPair ()
{
    if ( pub_key )
       RSA_free ( pub_key ) ;
  
    if ( pri_key )
    RSA_free ( pri_key ) ; 
}


int rsaPair::create_key_pair ()
{
  RSA *rsa ;
  int modulelen = 1024 ;
  int ret ;
  unsigned int len ;
  unsigned long e = RSA_3 ;
 
  BIGNUM *bn ;
  
  bn = BN_new () ; 
  ret = BN_set_word ( bn , e ) ;
   
  if ( ret != 1 )
  {
    perror ("BN_set_word method goes wrong ") ;
    return -1 ;
  }
  
  rsa = RSA_new () ;

  if ( RSA_generate_key_ex ( rsa , modulelen , bn , NULL ) != 1 )
  {
    perror ("RSA_generate_key_ex method goes wrong") ;
    return -1 ;
  }

//---------------------------------------------------------------
  
  // public key
  BIO *bioPtr = BIO_new ( BIO_s_file () ) ;
  
  //----- open public key store file ------
  if ( BIO_write_filename ( bioPtr , (void*)pub_key_path.c_str ()) <= 0 )
  {
    perror ("failed to open public key file ") ;
    return -1 ;
  }
    
  //----- write public key into file -----
 
  if ( PEM_write_bio_RSAPublicKey( bioPtr , rsa ) != 1 )
  {
     perror ("failed to write RSA public key into file") ;
    return -1 ;
  }

  //----- if we get here , everything goes well -----
  printf ("generated RSA public key already written into file %s \n" , pub_key_path.c_str()) ;
  
  BIO_free_all( bioPtr ) ; // don't forget release and free the allocated space


//-----------------------------------------------------------------------------------------
   //----- private key -----
    
   bioPtr = BIO_new_file ( pri_key_path.c_str() , "w+") ;
   
   if ( bioPtr == NULL )
   {
    perror ("failed to open file stores RSA private key ") ;
       return -1 ;
   } 

   if ( PEM_write_bio_RSAPrivateKey ( bioPtr , rsa ,EVP_des_ede3_ofb() ,
            (unsigned char *)password.c_str() , password.size() , NULL , NULL ) != 1 )
   {
    perror ("failed write RSA private key into file") ;
    return -1 ;
   }
    
   BIO_free_all ( bioPtr ) ; // do not forget this 
   
   printf ("genertated RSA private key already written into file %s \n" , pri_key_path.c_str () ) ;

  return 0 ;
}