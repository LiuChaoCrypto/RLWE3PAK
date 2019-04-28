#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <openssl/md5.h>


int get_md5(char *input,char *output)
{
    char password[1024*1024*5]={0};
    MD5_CTX x;
    int i=0;
    char *out=NULL;
    unsigned char d[16];
    strcpy(password,input);
    MD5_Init(&x);
    MD5_Update(&x,(char *)password,strlen(password));
    MD5_Final(d,&x);
    out=(char *)malloc(35);
    memset(out,0x00,35);
    strcpy(out,"$1$");

    for(i=0;i<16;i++)
    {
        sprintf(out+(i*2),"%02X",d[i]);
    }




    out[32]=0;

    strcpy(output,out);
    free(out);
    return 0;
}


int one_get_md5(char *input,uint32_t out_of_4ring[4],unsigned char d[16])
{
 //   char password[1024*1024*5]={0};
    MD5_CTX x;
    int i=0;


    for (i=0;i<16;i++)
    {
        d[i]=0;
    }
 //   strcpy(password,input);
    MD5_Init(&x);
    MD5_Update(&x,input,strlen(input));
    MD5_Final(d,&x);
    for(i=0;i<4;i++)
    {
    out_of_4ring[i]=(uint32_t)d[0+i*4]<<24|
        (uint32_t)d[1+i*4]<<16|
        (uint32_t)d[2+i*4]<<8|
        (uint32_t)d[3+i*4];
  //u  printf("\nhere is d :%I32u\n",out_of_4ring[i]);
    }
    return 0;
}

void full_get_md5(char *input,uint32_t ring[1024],int hashnumber)
{

    char password[1024*1024*5]={0};

    strcpy(password,input);
    password[0]=password[0]^hashnumber;
    uint32_t smallring[256][4];
    unsigned char d[16];
    for(int i=0;i<256;i++)
    {
         one_get_md5((char *)password,smallring[i],d);
        strcpy(password,(char *)d);
    }

    for(int i=0;i<256;i++)
    {
        for(int j=0;j<4;j++)
        {
            ring[i*4+j]=smallring[i][j];
        }
    }

}



void md5_hash_4ring_1k_to_hashk(const uint32_t m_A[1024],const uint32_t m_B[1024],const uint32_t p_A[1024], const uint32_t p_B[1024],const uint64_t k[16],int hashnumber,char* hashkout)
{
    char password[4*1024*4+8*16];
       for(int i=0;i<1024;i++)
   {
       sprintf(password+(i*4),"%02X",m_A[i]);
   }
    for(int i=0;i<1024;i++)
    {
        sprintf(password+4*1024+(i*4),"%02X",m_B[i]);
    }

       for(int i=0;i<1024;i++)
   {
       sprintf(password+2*4*1024+(i*4),"%02X",p_A[i]);
   }
    for(int i=0;i<1024;i++)
    {
        sprintf(password+3*4*1024+(i*4),"%02X",p_B[i]);

    }


       for(int i=0;i<16;i++)
       {
           sprintf(password+4*4*1024+(i*8),"%04X",k[i]);
       }
       password[0]=password[0]^hashnumber;
    get_md5(password,hashkout);
}


void md5_hash_1ring_1k_to_hashk(const uint32_t p[1024],const uint64_t k[16],int hashnumber,char* hashkout)
{
    char password[1024*4+8*16];
       for(int i=0;i<1024;i++)
   {
       sprintf(password+(i*4),"%02X",p[i]);
   }

       for(int i=0;i<16;i++)
       {
           sprintf(password+4*1024+(i*8),"%04X",k[i]);
       }
       password[0]=password[0]^hashnumber;
    get_md5(password,hashkout);
}


