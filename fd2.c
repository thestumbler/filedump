// #define DOS 0
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#if DOS
  #include <io.h>
#endif

#define MAXB 256
#define MXDEF 16

#define uchar unsigned char


int main( int argc, char *argv[] ) {
  int i,j,k,cnt,br;
  int ich;
  long foff,roff,off;
  int sr;
  int recno;
  int blkno;
  uchar dat[MAXB];
  uchar rt=0;
  uchar c;
  uchar line[256], line1[256], line2[256];
  int hs2;
  int rs=0;
  int hs=0;
  int bs=0;
  int vb=0;
  int fs=0;
  int lc=0;
  int mx=MXDEF;
  int rsmin=-1;
  int rsmax=-1;
  int rpt=0;
  int m1=-1;
  int m2=-2;


  ich = 0;
/*
  printf("File descriptor for stdin      %d\n", stdin->fd );
  printf("File descriptor for stdout     %d\n", stdout->fd );
  printf("File descriptor for stderr     %d\n", stderr->fd );
  printf("File descriptor for stdaux     %d\n", stdaux->fd );
*/


  for(i=1;i<argc;i++) { /* check for verbose mode */
    if( argv[i][0]=='-' ) {
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
        case 'o':
          if(sscanf(argv[i],"-o%lx",&off)!=1) {
            printf("ERROR: invalid offset specified: %s\n", argv[i]);
            exit(99);
          }
          if(vb) printf("Offset=0x%lx\n", off);
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
printf( "Usage: fd2 [-options] [filename] \n" );
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


  for(i=0;i<256;i++) {
    line[i]=0;
    line1[i]=0;
    line2[i]=0;
  }
  blkno=0;
  recno=0;
  foff=0;
  roff=0;
  off=0;
  i=0;
  hs2=hs;
  lc=0;
  sr=8;
  do {
    br=read(ich,&c,1);
    if( br == 1 ) {
      if( (i==0) && (roff==0) ) {
        if(hs2) {
          if(rs)      printf("Hdr:%03d, Off:0x%06lx (%ld)\n", recno, foff, foff );
          else if(rt) printf("Hdr:%03d, Off:0x%06lx (%ld)\n", recno, foff, foff);
          hs2--;
          if(hs2==0) recno=-1;
        } else if (bs) {
          if(rs)      printf("Blk:%03d, Rec:%03d, Off:0x%06lx (%ld)\n",blkno,recno,foff, foff );
          else if(rt) printf("Blk:%03d, Rec:%03d, Off:0x%06lx (%ld)\n",blkno,recno,foff, foff );
        } else {
          if(rs)      printf("Rec:%03d, Off:0x%06lx (%ld)\n", recno, foff, foff );
          else if(rt) printf("Rec:%03d, Off:0x%06lx (%ld)\n", recno, foff, foff );
        }
      }
      if(i==0) { sprintf((char *)line+lc, "%08lx: %n", off+roff, &k ); lc+=k; }
      dat[i]=c;
      sprintf((char *)line+lc, "%02x %n", dat[i], &k); lc+=k;
      i++;
    }
    if( (i>=mx) || (br!=1) || (rt&&(c==rt)) || (rs&&(roff+i>=rs)) ) {
      for(j=i;j<mx;j++) {
        sprintf((char *)line+lc, "   %n", &k); lc+=k;
      } /* print any trailing spaces */
      sprintf((char *)line+lc, "    %n", &k); lc+=k;
      for(j=0;j<i;j++) { /* print ascii characters */
        if( isprint( dat[j] )) { sprintf((char *)line+lc, "%c%n", dat[j], &k); lc+=k; }
        else { sprintf((char *)line+lc, ".%n", &k); lc+=k; }
      }
      roff += i;
      lc=0;
      i=0;
      m1 = strcmp( (char *)line+sr, (char *)line1+sr);
      m2 = strcmp( (char *)line+sr, (char *)line2+sr);
      strcpy( (char *)line2, (char *)line1 );
      strcpy( (char *)line1, (char *)line );
      if( (m1==0) && (m2==0) ) {
        rpt++;
      } else {
        if(rpt) printf("... (Line Repeats %d Times) ...\n", rpt);
        printf("%s\n",line);
        rpt=0;
      }
      if( (rt && (c==rt)) || (rs && (roff>=rs)) ) {
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
