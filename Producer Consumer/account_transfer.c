#include "stdio.h"
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "pthread.h"


#define Buff_Size 40

typedef struct Account acc;

struct Account
{
    int number,balence,status;
    acc* next;
};


acc* head;

char *total_in;
int str_size=0;
int no_acc=0;
int trans_start;
int num_threads;
int number_trans = 0;

pthread_mutex_t  checkacc_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t   acc_busy = PTHREAD_COND_INITIALIZER;


void read_input()
{
    int i,n;
    char input_buf[Buff_Size];
    int array_size=10;
    
    for(i=0;i<Buff_Size;i++)
        input_buf[i]=' ';

    while((n=read(STDIN_FILENO,input_buf,Buff_Size))>0)
    {
       if (n < Buff_Size)
       {
        input_buf[n]='\n';
        n++;
        input_buf[n]='\0';
       }
       str_size +=n;
       if(array_size<str_size)
       {
           total_in = (char *) realloc(total_in,str_size);
           array_size=str_size;
       }
       strcat(total_in,input_buf);
    }
}

void get_accounts()
{
    int i,k,valid,done,acc_no,bal,flag,start,j;
    char trans[]="Transfer ";
    char* str_process = (char*)malloc(100);
     acc* new_acc;
     acc* current;
    start = 0;
    for(i=start; i<str_size;i++)
      {
        flag=0;
        valid=1;
        done=1;
        acc_no=0;
        bal=0;
        if(total_in[i]=='\n')
        {
            if(i-start>100)
            {
                start=i+1;
                continue;
            }
            for(k = start, j=0;j<9;k++,j++)
            {
                 if(total_in[k]!=trans[j])
                {
                    done = 0;
                }
            }
            if(done)
            {
                trans_start=start;
                break;
            }
            no_acc++;
            for(k = start, j=0;k<=i;k++,j++)
            {
                 str_process[j] =  total_in[k];
                if(flag==0)
                {
                    if(str_process[j]==' ')
                    {
                        flag =1;
                        continue;
                    }
                    if((int)str_process[j]<48 ||(int)str_process[j]>57)
                    {
                     continue;
                    }
                    acc_no=acc_no*10+((int)str_process[j]-48);
                }
                else if(flag == 1)
                {
                    if((int)str_process[j]<48 ||(int)str_process[j]>57)
                    {
                     continue;
                    }
                    bal=bal*10+((int)str_process[j]-48);   
                }  
            }
            if(acc_no==0)
            {
                start=i+1;
                continue;
            }
            
            new_acc=malloc(sizeof(acc));
            new_acc->number = acc_no;
            new_acc->balence =  bal;
            new_acc->status = 0;
            new_acc->next=NULL;
            if(no_acc==1)
                head=new_acc;
            else
            {
                current=head;
                while(current->next!=NULL)
                {
                    current=current->next;
                }
                current->next = new_acc;
            }
            start=i+1;
        }
      }

    /* Used to print initial balences */
    /*
      current=head;
      while(current !=NULL)
        {
            printf("%i %i\n",current->number,current->balence);
            current=current->next;
        }
    */
}

void* process_transaction(void* args)
{
    char input[100];
    char* total_input_l;
    int from,to,value,k,i,j;
    int done=0;
    int flag = 0;
    int found =0;
    int start =0;
    char trans[]="Transfer ";
    acc *from_acc,*to_acc,*current;
    total_input_l= (char*)args;

    for(i=0;i<strlen(total_input_l);i++)
    {
    from =0;
    to=0;
    value = 0;
    done=0;
    flag=0;
    found=0;
    
    if(total_input_l[i]=='\n')
    {
        if(i-start>100)
            {
                start=i+1;
                continue;
            }
        for(k = start, j=0;k<=i;k++,j++)
           {
                input[j] =  total_input_l[k];
            }
            
    
    for(j=0;j<9;j++)
    {
         if(input[j]!=trans[j])
         {
            done = 1;
        }
    }
    if(!done)
    {
        for(j=9;j<=strlen(input);j++)
        {
                if(flag==0)
                {
                    if(input[j]==' ')
                    {
                        flag =1;
                        continue;
                    }
                    if((int)input[j]<48 ||(int)input[j]>57)
                    {
                     continue;
                    }
                    from=from*10+((int)input[j]-48);
                }
                else if(flag == 1)
                {
                    if(input[j]==' ')
                    {
                        flag =2;
                        continue;
                    }
                    if((int)input[j]<48 ||(int)input[j]>57)
                    {
                     continue;
                    }
                    to=to*10+((int)input[j]-48);   
                } 
                else
                {
                    if((int)input[j]<48 ||(int)input[j]>57)
                    {
                     continue;
                    }
                    value=value*10+((int)input[j]-48);
                }
        }
        
        pthread_mutex_lock(&checkacc_mtx);
        current =head; 
        while(current!=NULL && found !=2)
        {
            if(current->number == from)
            {
                from_acc = current;
                found++;
            }
            else if(current->number == to)
            {
                to_acc = current;
                found++;
            }
            current = current->next;
        }
        if(found!=2)
        {
            done=1;
        }
        if(!done)
        {
             while(from_acc->status!=0&&to_acc->status!=0)
            {
                 pthread_cond_wait(&acc_busy,&checkacc_mtx);
            }
            from_acc->status=1;
            to_acc->status=1;
        }
        pthread_mutex_unlock(&checkacc_mtx);
    }
    if(!done)
    {
        from_acc->balence-=value;
        to_acc->balence+=value;
    
        pthread_mutex_lock(&checkacc_mtx);
        from_acc->status=0;
        to_acc->status=0;
        pthread_cond_broadcast(&acc_busy);
        pthread_mutex_unlock(&checkacc_mtx);
    }
    start=i+1;
        
}
    }

}

int main(int argc, char *argv[])
{
    int s,i,j,k,start,iter,extra;
    total_in =(char *) malloc(10);
    acc* current;
    int last =0;
    int l=0;

    num_threads=0;
    if(argc!= 2)
    {
        printf("Invalid number of args \n");
        exit(-1);
    }
    for(i=0;i<strlen(argv[1]);i++)
    {
        if((int)argv[1][i]<48 ||(int) argv[1][i]>57)
        {
            printf("Invalid number of threads \n");
            exit(-1);
        }
        num_threads=num_threads*10+((int)argv[1][i]-48);
    }
    if(num_threads==0)
    {
        printf("Invalid buffer size \n");
        exit(-1);
    }
    
    pthread_t t1[num_threads];
    char* total_buff[num_threads];
    int buf_start[num_threads];
    
    read_input();

    get_accounts();

    start = trans_start;
    for(i=trans_start;i<strlen(total_in);i++)
    {
        if(total_in[i]=='\n')
        {
            number_trans++;
        }
      
    }

    extra  = number_trans%num_threads;
    
    for(i=0;i<num_threads;i++)
    {
        buf_start[i]=0;
        if(i<extra)
        {   
            total_buff[i]=malloc(100*(((int)number_trans/num_threads)+1));
        }
        else
        {
            total_buff[i]=malloc(100*((int)number_trans/num_threads));
        }
    }
    iter=0;
    for(i=trans_start;i<strlen(total_in);i++)
    {
        if(total_in[i]=='\n')
        {
        if(i-start>100)
            {
                start=i+1;
                continue;
            }
        for(k = start, j= buf_start[iter%num_threads];k<=i;k++,j++)
           {
                total_buff[iter%num_threads][j] =  total_in[k];
            }
            buf_start[iter%num_threads]=j;
            start=i+1;
            iter++;
        }
    }

    for(i=0;i<num_threads;i++)
    {
     pthread_create(&t1[i],NULL,process_transaction,(void*)total_buff[i]);
    }

    for(i=0;i<num_threads;i++)
    {
        pthread_join(t1[i],NULL);
    }
        
        
    current=head;
      while(current !=NULL)
        {
            printf("%i %i\n",current->number,current->balence);
            current=current->next;
        }

    return 0;
}