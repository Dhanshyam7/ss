#include<stdio.h>
 int n,m;
 void calculate_need(int max[][m],int allocated[][m],int need[][m])
 {
    for(int i=0;i<n;i++)
    {
        for(int j=0;j<m;j++)
        {
            need[i][j]=max[i][j]-allocated[i][j];
        }
    }
 }
 void display(int max[][m],int allocated[][m],int need[][m],int available[])
 {
    printf("PROCESS \tMAXIMUM\t\tALLOCATED\tNEED\t\tAVAILABLE\n\t\t");
    for(int i=0;i<m;i++)
    {
        printf("R%d ",i);
    }
    printf("\t");
    for(int i=0;i<m;i++)
    {
        printf("R%d ",i);
    }
    printf("\t");
    for(int i=0;i<m;i++)
    {
        printf("R%d ",i);
    }
    printf("\t");
    for(int i=0;i<m;i++)
    {
        printf("R%d ",i);
    }
    printf("\n");
    for(int i=0;i<n;i++)
    {
        printf("P%d \t\t",i);
        for(int j=0;j<m;j++)
        {
            printf("%d  ",max[i][j]);
        }
        printf("\t");
        for(int j=0;j<m;j++)
        {
            printf("%d  ",allocated[i][j]);
        }
        printf("\t");
        for(int j=0;j<m;j++)
        {
            printf("%d  ",need[i][j]);
        }
        if(i==0)
        {
            printf("\t");
            for(int j=0;j<m;j++)
            {
                printf("%d ",available[j]);
            }
        }
        printf("\n");
    }
 }
 int check_request(int request[],int p,int need[][m],int available[])
 {
    for(int j=0;j<m;j++)
    {
        if(request[j]>need[p][j])
        {
            return 0;
            
        }
        if(request[j]>available[j])
        {
            return 0;
           
        }
    }
    return 1;
 }
 void deadlock_detect(int max[][m],int allocated[][m],int need[][m],int 
available[m])
 {
    int safe[n],work[m],finish[n];
    for(int i=0;i<m;i++)
    {
        work[i]=available[i];
    }
    for(int i=0;i<n;i++)
    {
        finish[i]=0;
    }
    int count=0,j=0,i=0,found=0;
    while(1)
    {
        found=0;
        for(i=0;i<n;i++)
        {
            if(finish[i]==0)
            {
                for(j=0;j<m;j++)
                {
                    if(need[i][j]>work[j])
                    {
                        break;
                    }
                }
                if(j==m)
                {
                    safe[count++]=i;
                    finish[i]=1;
                    found=1;
                    for(int k=0;k<m;k++)
                    {
                        work[k]+=allocated[i][k];
                    }
                }
            }
        }
        if(found==0)
        {
            break;
        }
        
    }
    if(count==n)
    {
        printf("Printing the safe sequence\n");
        for(int i=0;i<n;i++)
        {
            printf("P%d ",safe[i]);
        }
        printf("\n");
    }
    else
    {
        printf("NO SAFE SEQUENCE DEADLOCK...\n");
    }
 }
 int main()
 {
    printf("Enter number of process-");
    scanf("%d",&n);
    printf("Enter no of resources-");
    scanf("%d",&m);
    int max[n][m],allocated[n][m],need[n][m];
    for(int i=0;i<n;i++)
    {
        printf("Enter maximum and allocated resource of each type for process P%d\n",i);
        for(int j=0;j<m;j++)
        {
            scanf("%d",&max[i][j]);
        }
        for(int j=0;j<m;j++)
        {
            scanf("%d",&allocated[i][j]);
        }
    }
    int available[m];
    printf("Enter available resource of each type\n");
    for(int i=0;i<m;i++)
    {
        scanf("%d",&available[i]);
    }
    calculate_need(max,allocated,need);
    char ch='n';
    printf("Enter y if there is any request-");
    scanf(" %c",&ch);
    while(ch=='y')
    {
        int p,y;
        printf("\nEnter process for which the request is-");
        scanf("%d",&p);
        int request[m];
        printf("\nEnter the request\n");
        for(int j=0;j<m;j++)
        {
            scanf("%d",&request[j]);
        }
        y=check_request(request,p,need,available);
        if(y==0)
        {
            printf("INVALID REQUEST\n");
        }
        else
        {
            for(int j=0;j<m;j++)
            {
                allocated[p][j]+=request[j];
                available[j]-=request[j];
            }
        }
        ch='n';
    }
    calculate_need(max,allocated,need);
    deadlock_detect(max,allocated,need,available);
    display(max,allocated,need,available);
 }
