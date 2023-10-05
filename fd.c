/*

Terminology Used for (f)ile (d)ump utility:

Assumed File Structure:

  file {
    header{
      data_byte[0..hs]
    }
    block[0] {
      record[0] { data_byte[0..rs] }
      record[1] { data_byte[0..rs] }
      record[2] { data_byte[0..rs] }
        ....
      record[bs] { data_byte[0..rs] }
    } -end of block-
  }


*/

#define DOS 1
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#if DOS
  #include <io.h>
#endif

#define MAXB 256
#define MXDEF 16

#define uchar unsigned char


void main( int argc, char *argv[] )
{
  int i,j,cnt,br;
  int ich;
  long foff,roff;
  int recno;
  int blkno;
  uchar dat[MAXB];
  uchar rt=0;
  uchar c;
  int hs2;
  int rs=0;
  int hs=0;
  int bs=0;
  int vb=0;
  int fs=0;
  int mx=MXDEF;
  int rsmin=-1;
  int rsmax=-1;
  int vbrec=1;
  int vbblk=1;
  int vdhdr=1;


  ich = 0;
/*
  printf("File descriptor for stdin     %d\n", stdin->fd );
  printf("File descriptor for stdout     %d\n", stdout->fd );
  printf("File descriptor for stderr     %d\n", stderr->fd );
  printf("File descriptor for stdaux     %d\n", stdaux->fd );
*/


  for(i=1;i<argc;i++) { /* check for verbose mode */
    if( argv[i][0]=='-' && strlen(argv[i])==2 ) {
      if( argv[i][1]=='v' ) vb=1;
      if( argv[i][1]=='V' ) vb=1;
    }
  }

  for(i=1;i<argc;i++) {
    if( argv[i][0]=='-' || argv[i][0]=='/' ) { /* check for switch */
      switch( argv[i][1] ) {
        case 't':
          rt='\n';
          if(vb) printf("Record Terminator: %02x\n", rt );
          break;
        case 'z':
          if(sscanf(argv[i],"-z%d",&rs)!=1) {
            printf("ERROR: invalid record size specified: %s\n", argv[i]);
            exit(99);
          }
          if(vb) printf("Record size=%d\n", rs);
          break;
        case 'h':
          if(sscanf(argv[i],"-h%d",&hs)!=1) {
            printf("ERROR: invalid header size specified: %s\n", argv[i]);
            exit(99);
          }
          if(vb) printf("Header size=%d\n", hs);
          break;
        case 'b':
          if(sscanf(argv[i],"-b%d",&bs)!=1) {
            printf("ERROR: invalid block size specified: %s\n", argv[i]);
            exit(99);
          }
          if(vb) printf("Block size=%d\n", bs);
          break;
        case 'x':
          if(sscanf(argv[i],"-x%d",&mx)!=1) {
            printf("ERROR: invalid max display size specified: %s\n", argv[i]);
            exit(99);
          }
          if((mx<=0)||(mx>MAXB)) {
            printf("ERROR: invalid max display size specified: %d\n", mx);
            exit(99);
          }
          if(vb) printf("Max Display Size=%d\n", mx);
          break;
        case 'v': 
          if(vb) printf("Verbose Mode\n");
          break;
        case 's': 
          if(vb) printf("Statistics Mode\n");
          fs=1;
          break;
        case '?':
        default:
printf( "Usage: fd [-options] [filename] \n" );
printf( "Options:\n" );
printf( "    -t          Variable Records, Terminator = newline\n" );
printf( "    -z[value]   Fixed Size Records = [value] bytes per record\n" );
printf( "    -h[value]   Header size - [value] number of records in header\n" );
printf( "    -b[value]   Block Size = [value] records per block\n" );
printf( "    -x[value]   maX bytes per display line\n" );
printf( "    -v          Verbose mode - echo input parameters\n" );
printf( "    -s          Statistics are collected on file\n" );
printf( "  [Filename]    If specified, otherwise from standard input\n" );
          exit(0);
      }
    } else { /* assume input filename specified */
      ich = open( argv[i], O_RDONLY 
            #if DOS
              | O_BINARY
            #endif
            );
      if( ich == -1 ) {
        printf("ERROR opening file: %s\n", argv[i] );
        exit(99);
      }
    }
  }


  blkno=0;
  recno=0;
  foff=0;
  roff=0;
  i=0;
  hs2=hs;
  do {
    br=read(ich,&c,1);
    if( br == 1 ) {
      if( (i==0) && (roff==0) ) {
        if(hs2) {
          if(rs)      printf("Hdr:%03d, Off:0x%06lx (%ld)\n", recno, foff, foff );
          else if(rt) printf("Hdr:%03d, Off:0x%06lx (%ld)\n", recno, foff, foff);
        } else if (bs) {
          if(rs)      printf("Blk:%03d, Rec:%03d, Off:0x%06lx (%ld)\n",blkno,recno,foff, foff );
          else if(rt) printf("Blk:%03d, Rec:%03d, Off:0x%06lx (%ld)\n",blkno,recno,foff, foff );
        } else {
          if(rs)      printf("Rec:%03d, Off:0x%06lx (%ld)\n", recno, foff, foff );
          else if(rt) printf("Rec:%03d, Off:0x%06lx (%ld)\n", recno, foff, foff );
        }
      }
      if(i==0) printf("%06lx: ", roff );
      dat[i]=c;
      printf("%02x ", dat[i]);
      i++;
    }
    if( (i>=mx) || (br!=1) || (rt&&(c==rt)) || (rs&&!hs2&&(roff+i>=rs)) ||
        (hs2&&(roff+i>=hs)) ) {
      for(j=i;j<mx;j++) printf("   "); /* print any trailing spaces */
      printf("    ");
      for(j=0;j<i;j++) { /* print ascii characters */
        if( isprint( dat[j] )) printf("%c", dat[j]);
        else printf(".");
      }
      roff += i;
      i=0;
      printf("\n");
      if( (rt && (c==rt)) || (rs&&!hs2 && (roff>=rs)) || (hs2 && (roff>=hs)) ) {
        recno++;
        if( bs && !hs2 ) { 
          if(recno>=bs) blkno++;
          recno %= bs;
        }
        foff+=roff;
        if( fs && (roff!=0) ) {
          if( (rsmax==-1) || (roff>rsmax) ) rsmax=roff;
          if( (rsmin==-1) || (roff<rsmin) ) rsmin=roff;
        }
        if( hs2 && (roff>=hs)) {
          hs2=0;
          recno=0;
        }
        roff=0;
      }
    }
  } while( br > 0 );

  if(fs) {
    printf("File Size: 0x%lx (%ld)\n", foff, foff );
    if(hs) printf("Number of Header Records: %d\n", hs );
    if(bs) printf("Number of %d Record Blocks: %d\n", bs, recno );
    else   printf("Number of Records: %d\n", recno );
    if(rt) {
      printf("Maximum Record Size: %d\n", rsmax );
      printf("Minimum Record Size: %d\n", rsmin );
    }
  }

  close(ich);
  
}  
