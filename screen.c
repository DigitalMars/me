/*_ SCREEN.C	10-Aug-83	*/
/* Test mono or color screen attributes	*/

#include	<stdio.h>
#include	<windows.h>
#include        <stdlib.h>

int main()
{
  COORD sbcoord;
  COORD sbsize;
  SMALL_RECT sdrect;
  CHAR_INFO sbbuf[16 + 1 + 16];
  HANDLE hStdout;

  hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

  sbcoord.X = 0;
  sbcoord.Y = 0;
  sbsize.X = 16+1+16;
  sbsize.Y = 16;

  for (int row = 0; row < 16; row++)
  {
	sdrect.Left = 0;
	sdrect.Top = 2 + row;
	sdrect.Right = sdrect.Left + 16+1+16;
	sdrect.Bottom = sdrect.Top + 0;

	for (int col = 0; col < 16; col++)
	{
	    int i = row * 16 + col;
	    sbbuf[col].Char.AsciiChar = i;
	    sbbuf[col].Attributes = 0x07;
	    sbbuf[16 + 1 + col].Char.AsciiChar = 'A';
	    sbbuf[16 + 1 + col].Attributes = i;
	}
	sbbuf[16].Char.AsciiChar = (row > 9) ? row - 10 + 'a' : row + '0';
	sbbuf[16].Attributes = 0xF0;
	WriteConsoleOutput(hStdout,sbbuf,sbsize,sbcoord,&sdrect);
  }

  return 0;
}
