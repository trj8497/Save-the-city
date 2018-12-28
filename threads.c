///File name: threads.c
///Author's name: Tejaswini Jagtap
///Date: 12/7/2018
///This project make use of curses, multi threading to build a game that 
///accepts user input while playing the game.

#define _DEFAULT_SOURCE

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/ioctl.h>
#include<ncurses.h>
#include<unistd.h>
#include<ctype.h>
#include<pthread.h>

int height, width, def_row, missiles, flag, temp_row, temp_col;  

char** pos_status; //2-D array that stores position status for each position on the screen 
char attack[81];   //array to store the name of attacker 
char defence[81];  //array to store the name of defenser 
pthread_mutex_t p_mutex = PTHREAD_MUTEX_INITIALIZER;

///This is the structure for defense sheild. 
typedef struct Defense_S
{
   int row;
   int col;
   char graphic[5];
}Defense;

Defense * defense;


///This is the structure for missile(single missile).
typedef struct Missile_S
{
   int row;
   int col;
   char* graphic;
}Missile;

///This function builds the cityscape on a different window.
///@param layout: array that contains all the city layout information
void config_building(int * layout)
{
   initscr();
   cbreak();
   int i;
   move(0,0);
   printw("Enter 'q' to quit at the end of attack, or control-C");
   refresh();
   for (i = 0; i < width; i++) 
   {
      if (layout[i] == 2) 
      {
         pos_status[height-2][i] = '2';
         move(height-2, i);
         printw("_");
         refresh();
      } 
      else 
      {
         int sameNum = 1, previous, next, l, check = 0, current;
         for (l = i; l < width-1; l++) 
         {
            if (layout[i] == layout[l+1])
               sameNum++;
            else
               break;
         }   
         if (l < width-1) 
         {
            next = layout[l+1];
         }
         else
         {
            next = 2;
         }
         if (i > 0) 
         {
            if (layout[i] == layout[i-1]) 
            {
               sameNum++; 
               previous = layout[i-2];
            }
            else 
            {
               previous = layout[i-1];
            }
         }  
         else
         { 
            previous = 2;
         }
         current = i;
         int z;                
         if (previous > layout[i]) 
         {
            if (next > layout[current]) 
            {
               for (int x = 0; x <= sameNum-1; x++) 
               {
                  pos_status[height-layout[current]][i] = 'h';
                  move (height-layout[current], i);
                  printw("_");
                  refresh();
                  i++;
               }
               for (int x = 2; x < layout[i]; x++)
               {
                  pos_status[height-x][i] = 'b';
                  move(height-x, i);
                  printw("|");
                  refresh();
               }
            }
            else 
            {
               for (int x = 0; x <= sameNum-2; x++) 
               {
                  pos_status[height-layout[current]][i] = 'h';
                  move(height-layout[current], i);
                  printw("_");
                  refresh();
                  i++;
               }
               for (int x = 2; x < layout[i]; x++)
               {
	          pos_status[height-x][i] = 'b';
                  move(height-x, i);
                  printw("|");
                  refresh();
               }     
            }   
         } 
         else
         {
            if (layout[i] != layout[i-1]) 
            {
               for (z = 2; z < layout[i]; z++) 
               {
	          pos_status[height-z][i] = 'b';
                  move(height-z, i);
                  printw("|");
                  refresh();
               }
               i++;
            }
            else
            {
               check = 1;
               if (next < layout[current])
               {
                  for (int x = 0; x < sameNum-2; x++)
                  {
	             pos_status[height-layout[current]][i] = 'h';
                     move(height-layout[current], i);
	             printw("_");
	             refresh();
	             i++;
                  }
                  if (layout[i] == layout[current])
                  {
                     for (z = 2; z < layout[i]; z++)
                     {
		        pos_status[height-z][i] = 'b';
	                move(height-z, i);
	                printw("|");
	                refresh();
                     }
	          }
	       }
	       else
	       {
                  for (int x = 0; x < sameNum-1; x++)
                  {
	             pos_status[height-layout[current]][i] = 'h';
	             move(height-layout[current], i);
	             printw("_");
	             refresh();
	             i++;
	          }
                  for (int x = 2; x < layout[i]; x++)
                  {
	             pos_status[height-x][i] = 'b';
	             move(height-x, i);
	             printw("|");
	             refresh();
                  }
	       }
	    }
	    if (check == 1)
               continue;
            if (next > layout[current])
	    {
               for (int x = 0; x < sameNum-1; x++)
	       {
	          pos_status[height-layout[current]][i] = 'h';
                  move (height-layout[current], i);
	          printw("_");
	          refresh();
                  i++;
	       }
	       for (int x = 2; x < layout[i]; x++)
	       {
	          pos_status[height-x][i] = 'b';
                  move(height-x, i);
	          printw("|");
	          refresh();
	       }
            }
            else
            {
	       for (int x = 0; x < sameNum-2; x++)
               {
	          pos_status[height-layout[current]][i] = 'h';
                  move(height-layout[current], i);
	          printw("_");
	          refresh();
	          i++;
               }	
               for (int x = 2; x < layout[i]; x++)
               {
	          pos_status[height-x][i] = 'b';
                  move(height-x, i);
	          printw("|");
                  refresh();
               }
            }
         }  
      }
   }
}

///This is the set up function for the defense sheild thread.
///@param row: row of the defense sheild is fixed.
///@param col: col will be variable as defense sheild is free to move left or
///right
///@return: Defense structure 
Defense* make_defense (int row, int col)
{
   Defense* defense;
   defense = (Defense*)malloc(sizeof(Defense));
   defense->row = row;
   defense->col = col;
   strcpy(defense->graphic, "#####");
   return defense;
}

///Run function for the defense thread that is it has all the functionality 
///of the defense thread.
void *run1()
{
   noecho();
   cbreak();
   defense  = make_defense(def_row, width/2);
   temp_row = defense->row;
   temp_col = defense->col;
   pthread_mutex_lock(&p_mutex);
   move(defense->row, defense->col);
   printw(defense->graphic);
   refresh();
   pthread_mutex_unlock(&p_mutex);
   int c;
   while (1)
   { 
      c = getch();
      if ((c == 68) && (((defense->col)-1) >= 0))
      { 
         temp_row = defense->row;
         temp_col = defense->col;
         pthread_mutex_lock(&p_mutex);
         move(defense->row, defense->col);
         printw("     ");
	 refresh();
         pthread_mutex_unlock(&p_mutex);
	 (defense->col)--;
	 temp_row = defense->row;
         temp_col = defense->col;
         pthread_mutex_lock(&p_mutex);
         move(defense->row, defense->col);
         printw(defense->graphic);
	 refresh();
         pthread_mutex_unlock(&p_mutex);
      }
      else if ((c == 67) && ((defense->col)+1) < (width-4))
      {
         temp_row = defense->row;
         temp_col = defense->col;
         pthread_mutex_lock(&p_mutex);
         move(defense->row, defense->col);
         printw("     ");
	 refresh();
         pthread_mutex_unlock(&p_mutex);
	 (defense->col)++;
	 temp_row = defense->row;
         temp_col = defense->col;
	 pthread_mutex_lock(&p_mutex);
         move(defense->row, defense->col);
         printw(defense->graphic);
	 refresh();
         pthread_mutex_unlock(&p_mutex);
      }
      else if (c == 'q')
      {
         flag = 1; 
         break;
      }
   }
   return NULL;
}

///This is the set up function for a singel missile.
///@param row: row of the missile 
///@param col: col of the missile 
Missile* make_missile (int row, int col)
{
   Missile* missile;
   missile = (Missile*)malloc (sizeof (Missile)); 
   missile-> row = row;
   missile->col = col;
   missile->graphic = "|";
   return missile;
}

///The run function missile tnat is this function has aal the functionality 
///for a singel missile.
void* run2 ()
{
   Missile* missile;
   int r = rand() % width;
   missile = make_missile (2, r);
   int row = 2;
   while (row <= height-2)
   {
      if (flag == 1)
      {
         if ((temp_row == (missile->row)+1) && (temp_col <= missile->col) && ((temp_col)+5 > missile->col))
         {
            pthread_mutex_lock(&p_mutex);
            move(missile->row, missile->col);
            printw("?");
            refresh();
            move(missile->row+1, missile->col);
            printw("*");
            refresh();
            pthread_mutex_unlock(&p_mutex);
            break;
         }
      }
      else if ((defense->row == (missile->row)+1) && (defense->col <= missile->col) && ((defense->col)+5 > missile->col))
      {
         pthread_mutex_lock(&p_mutex);
         move(missile->row, missile->col);
         printw("?");
         refresh();
         move(missile->row+1, missile->col);
         printw("*");
         refresh();
         pthread_mutex_unlock(&p_mutex);
         break;
      }
      else if (pos_status[missile->row][missile->col] == 'h')
      {
         pos_status[missile->row][missile->col] = 'o';
	 if ((missile->row)+1 == height-2)
	 {
	    pos_status[(missile->row)+1][missile->col] = '2';
	 }
	 else
	 {
	    pos_status[(missile->row)+1][missile->col] = 'h';
	 }
	 pthread_mutex_lock(&p_mutex);
         move(missile->row, missile->col);
         printw("?");
         refresh();
         move((missile->row)+1, missile->col);
         printw("*");
         refresh();
         pthread_mutex_unlock(&p_mutex);
         break;
      }
      else if (pos_status[missile->row][missile->col] == 'b')
      {
         pos_status[missile->row][missile->col] = 'o';
	 if ((missile->row)+1 == height-2)
	    pos_status[(missile->row)+1][missile->col] = '2';
	 else
            pos_status[(missile->row)+1][missile->col] = 'b';
         pthread_mutex_lock(&p_mutex);
         move((missile->row)-1, missile->col);
         printw("?");
         refresh();
         move((missile->row), missile->col);
         printw("*");
         refresh();
         pthread_mutex_unlock(&p_mutex);
         break;
      }
      else if (pos_status[missile->row][missile->col] == '2')
      {
         pthread_mutex_lock(&p_mutex);
         move((missile->row), missile->col);
         printw("?");
         refresh();
         move((missile->row)+1, missile->col);
         printw("*");
         refresh();
         pthread_mutex_unlock(&p_mutex);
	 break;
      }
      else
      {
         pthread_mutex_lock(&p_mutex);
         move(missile->row, missile->col);
         printw(missile->graphic);
         refresh();
         pthread_mutex_unlock(&p_mutex);
      }
      int rd = rand() % 10;
      usleep(80000*rd);
      pthread_mutex_lock(&p_mutex);
      move(missile->row, missile->col);
      printw(" ");
      refresh();
      pthread_mutex_unlock(&p_mutex);
      row++;
      (missile->row)++;
   }
   free(missile);
   return NULL;
}

///This function joins the missiles number of missile threads. And if the
///number of missiles is 0 then the infinite threads are created.  
void* run3()
{
  pthread_t thred_arr[missiles];
  if (missiles != 0)
  {
     for (int i = 0; i < missiles; i++)
     {
        pthread_create (&thred_arr[i], NULL, run2, NULL);
        sleep(1);
     }
     for (int i = 0; i < missiles; i++)
     {
        pthread_join(thred_arr[i], NULL);
     }
     move(2, 0);
     printw("The %s attack has ended.", attack);
     refresh();
  }
  else
  {
     while (1)
     {
        pthread_t thred;
	pthread_create(&thred, NULL, run2, NULL);
	sleep(1);
     }
  }
  if (pos_status != NULL)
  {
     for (int a = 0; a < height; a++)
     {
        if (pos_status[a] != NULL)
        {
           free(pos_status[a]);
        }
     }
     free(pos_status);
  }
  return NULL;
}

///The main function reads the file and stores the information. It also 
///creates the defense and missiles threads and join them. Main function 
///also takes care of error handleing.
int main(int argc, char *argv[])
{
   if (argc != 2)
   {
      fprintf(stderr, "Usage: threads game-file\n");
      exit(EXIT_FAILURE);
   }
   struct winsize w;
   ioctl (0, TIOCGWINSZ, &w);
   width = w.ws_col;
   height = w.ws_row;
   FILE *fp;
   char *missile = malloc(1000 * sizeof(char));
   int *layout = malloc(width * sizeof(int));
   int count = 0; 
   int c;
   fp = fopen(argv[1], "r");
   if (fp == NULL)
   {
      perror(argv[1]);
      if (missile != NULL)
         free(missile);
      if (layout != NULL)
         free(layout);
      exit(EXIT_FAILURE);
   }
   int cor = 0;
   while ((c = fgetc(fp)) != EOF)
   {
      cor = 1;
      if (c == '#')
      {
         while ((c = fgetc(fp)) != '\n'){}
      }
      else
      {
         if (count == 0)
         {
	    int chk = 0;
            if (c == ' ')
            {
               while ((c = fgetc(fp)) != ' ');
            }
            int i = 0;
	    if (!isspace(c))
	       chk = 1;
            defence[i] = (char)c;
            i++;
            while ((c = fgetc(fp)) != '\n' && c != EOF)
            {
               defence[i] = (char)c;
               i++;
            }
 	    defence[i] = '\0';
	    if (chk == 0)
               break;
	    count++;
	    if (c == EOF)
               break;
         }
         else if (count == 1)
         {
	    int chk = 0;
            if (c == ' ')
            {
               while ((c = fgetc(fp)) != ' ');
            }
            int j = 0;
	    if (!(isspace(c)))
	       chk = 1;
            attack[j] = (char)c;
            j++;
            while ((c = fgetc(fp)) != '\n' && c != EOF)
            {
               attack[j] = (char)c;
               j++;
            }
	    attack[j] = '\0';
	    if (chk == 0)
	       break;
            count++;
	    if (c == EOF)
	       break;
         }
         else if (count == 2)
         {
            int k = 0;
            if (c == ' ')
            {
               while ((c = fgetc(fp)) != ' ');
            }
            if (c == '-')
            {
               fprintf(stderr, "Error: missile specification < 0.\n");
               fclose(fp);
	       if (missile != NULL)
                  free(missile);
	       if (layout != NULL)
                  free(layout);
               exit(EXIT_FAILURE);
            }
            if (isdigit(c))
            {
               missile[k] = c;
               k++;
               count++;
            }
            while (((c = fgetc(fp)) != '\n' && c != ' ') && c != EOF)
            {  
               if (isdigit(c))
               {
                  missile[k] = c;
                  k++;
               }
            }
	    if (k < 1000)
	       missile[k] = '\0';
            if (c == ' ')
            {
               while ((c = fgetc(fp)) != '\n');
            }
	    missiles = atoi(missile);
	    if (c == EOF)
	    {
	       break;
	    }
         }
         else if (count == 3)
         {
            if (c == ' ')
            {
               while ((c = fgetc(fp)) != ' ');
            }
            char *arr = malloc(height * sizeof(char));
            int l = 0;
            int h = 0; 
	    count++;
            if (isdigit(c))
            {
               arr[h] = c; 
               h++;
            }
            while ((c = fgetc(fp)) != ' ' && c != EOF)
            {
               if (isdigit(c))
               {
                  arr[h] = c; 
                  h++;
               }
            }
	    if (h < height)
	       arr[h] = '\0';
	    if (arr[0] != '\0')
	    {
	       layout[l] = atoi(arr);
               l++;
	    }
	    if (arr != NULL)
               free(arr);
            while ((l < width) && (c = fgetc(fp)) != EOF)
            {
               if (c != '\n' && c != ' ')
               {
                  char* arr1 = malloc(height * sizeof(char));
                  h = 0;
                  if (isdigit(c))
                  {
                     arr1[h] = c; 
                     h++;
                  }
                  while ((c = fgetc(fp)) != ' ' && c != '\n' && c != EOF)
                  {
                     if (isdigit(c))
                     {
                        arr1[h] = c; 
                        h++;
                     }
                  }
	          if (h < height)
	             arr1[h] = '\0';
	          if (arr1[0] != '\0')
	          {
	             layout[l] = atoi(arr1);
                     l++;
	          }
	          if (arr1 != NULL)
                     free(arr1);
               }
            }
            if (l < (width-1))
            {
               for (int n = l; n < width; n++)
               {
                  layout[n] = 2;
               }
	       break;
            }
         }
      }
   }
   if (c == EOF && cor == 0)
   {
      fprintf(stderr, "Error: missing defender name.\n");
      if (missile != NULL)
         free(missile);
      if (layout != NULL)
         free(layout);
      fclose(fp);
      exit(EXIT_FAILURE);
   }
   if (count == 0)
   {
      fprintf(stderr, "Error: missing defender name.\n");
      fclose(fp);
      if (missile != NULL)
         free(missile);
      if (layout != NULL)
         free(layout);
      exit(EXIT_FAILURE);
   }
   if (count == 1)
   {
      fprintf(stderr, "Error: missing attacker name.\n");
      if (missile != NULL)
         free(missile);
      if (layout != NULL)
         free(layout);
      fclose(fp);
      exit(EXIT_FAILURE);
   }
   if (count == 2)
   {
      fprintf(stderr, "Error: missing missile specification.\n");
      fclose(fp);
      if (missile != NULL)
         free(missile);
      if (layout != NULL)
         free(layout);
      exit(EXIT_FAILURE);
   }  
   if (count == 3)
   {
      fprintf(stderr, "Error: missing city layout.\n");
      fclose(fp);
      if (missile != NULL)
         free(missile);
      if (layout != NULL)
         free(layout);
      exit(EXIT_FAILURE);
   }
   pos_status = malloc (sizeof(char*)*height);
   for (int t = 0; t < height; t++)
   {
      pos_status[t] = malloc (sizeof(char)*width);
   }
   for (int a = 0; a < height; a++)
   {
      for (int b = 0; b < width; b++)
      {
         pos_status[a][b] = 'o';
      }
   }
   for (int a = 0; a < width; a++)
   {
      if (def_row < layout[a])
      {
         def_row = layout[a];
      }
   }
   def_row = height-def_row-4;
   config_building(layout);
   pthread_t def;
   pthread_t attacker;
   pthread_create(&def, NULL, run1, NULL);
   sleep(1);
   pthread_create(&attacker, NULL, run3, NULL);
   pthread_join(attacker, NULL);
   pthread_join(def, NULL);
   if (flag == 1)
   {
      move(4, 0);
      printw("The %s defense has ended.", defence);
      refresh();
      move(5,0);
      printw("hit enter to close...");
      refresh();
   }
   else
   {
      int h = getch();
      if (h == 'q')
      {
         endwin();
      }
   }
   if (defense != NULL)
      free(defense);
   move(height-1, width-1);
   fclose(fp);
   refresh();
   getch();
   endwin();
   if (missile != NULL)
      free(missile);
   if (layout != NULL)
      free(layout);
   return 0;
}
