#include <stdio.h>
#include <stdlib.h>

struct Process {
    int pid, at, bt, pri;
};

// Utility: Swap function for sorting
void swap(struct Process *a, struct Process *b) {
    struct Process t = *a;
    *a = *b;
    *b = t;
}

// Utility: Sort by Arrival Time
void sortByArrival(struct Process p[], int n) {
    for (int i = 0; i < n-1; i++) {
        for (int j = 0; j < n-i-1; j++) {
            if (p[j].at > p[j+1].at)
                swap(&p[j], &p[j+1]);
        }
    }
}

// ---------- FCFS ----------
void FCFS(struct Process p[], int n) {
    sortByArrival(p, n);
    int wt[n], tat[n], totalWT=0, totalTAT=0;

    wt[0] = 0;
    for (int i = 1; i < n; i++) {
        int finish_time_prev = p[i-1].at + wt[i-1] + p[i-1].bt;
        if (finish_time_prev < p[i].at)
            wt[i] = 0; // CPU idle
        else
            wt[i] = finish_time_prev - p[i].at;
    }

    for (int i = 0; i < n; i++) {
        tat[i] = wt[i] + p[i].bt;
        totalWT += wt[i];
        totalTAT += tat[i];
    }

    printf("\n--- FCFS Scheduling ---\n");
    printf("PID\tAT\tBT\tWT\tTAT\n");
    for (int i = 0; i < n; i++)
        printf("P%d\t%d\t%d\t%d\t%d\n", p[i].pid, p[i].at, p[i].bt, wt[i], tat[i]);

    printf("Average WT = %.2f\n", (float)totalWT/n);
    printf("Average TAT = %.2f\n", (float)totalTAT/n);
}

// ---------- SJF (Non-preemptive) ----------
void SJF(struct Process p[], int n) {
    int wt[n], tat[n], done[n], totalWT=0, totalTAT=0;
    for (int i=0;i<n;i++) done[i]=0;

    int time=0, completed=0;
    while (completed<n) {
        int idx=-1, minBT=1e9;
        for (int i=0;i<n;i++) {
            if (!done[i] && p[i].at<=time && p[i].bt<minBT) {
                minBT=p[i].bt;
                idx=i;
            }
        }
        if (idx==-1) { time++; continue; } // CPU idle

        wt[idx]=time-p[idx].at;
        if (wt[idx]<0) wt[idx]=0;
        tat[idx]=wt[idx]+p[idx].bt;
        time+=p[idx].bt;
        done[idx]=1;
        completed++;
    }

    printf("\n--- SJF Scheduling ---\n");
    printf("PID\tAT\tBT\tWT\tTAT\n");
    for (int i=0;i<n;i++) {
        printf("P%d\t%d\t%d\t%d\t%d\n",p[i].pid,p[i].at,p[i].bt,wt[i],tat[i]);
        totalWT+=wt[i]; totalTAT+=tat[i];
    }
    printf("Average WT = %.2f\n", (float)totalWT/n);
    printf("Average TAT = %.2f\n", (float)totalTAT/n);
}

// ---------- Priority (Non-preemptive) ----------
void PriorityScheduling(struct Process p[], int n) {
    int wt[n], tat[n], done[n], totalWT=0, totalTAT=0;
    for (int i=0;i<n;i++) done[i]=0;

    int time=0, completed=0;
    while (completed<n) {
        int idx=-1, bestPri=1e9;
        for (int i=0;i<n;i++) {
            if (!done[i] && p[i].at<=time && p[i].pri<bestPri) {
                bestPri=p[i].pri;
                idx=i;
            }
        }
        if (idx==-1) { time++; continue; } // CPU idle

        wt[idx]=time-p[idx].at;
        if (wt[idx]<0) wt[idx]=0;
        tat[idx]=wt[idx]+p[idx].bt;
        time+=p[idx].bt;
        done[idx]=1;
        completed++;
    }

    printf("\n--- Priority Scheduling ---\n");
    printf("PID\tAT\tBT\tPRI\tWT\tTAT\n");
    for (int i=0;i<n;i++) {
        printf("P%d\t%d\t%d\t%d\t%d\t%d\n",p[i].pid,p[i].at,p[i].bt,p[i].pri,wt[i],tat[i]);
        totalWT+=wt[i]; totalTAT+=tat[i];
    }
    printf("Average WT = %.2f\n", (float)totalWT/n);
    printf("Average TAT = %.2f\n", (float)totalTAT/n);
}

// ---------- Round Robin ----------
void RoundRobin(struct Process p[], int n, int quantum) {
    int remBT[n], wt[n], tat[n], totalWT=0, totalTAT=0;
    for (int i=0;i<n;i++) remBT[i]=p[i].bt;

    int time=0, completed=0;
    while (completed<n) {
        int doneCycle=1;
        for (int i=0;i<n;i++) {
            if (remBT[i]>0 && p[i].at<=time) {
                doneCycle=0;
                if (remBT[i]>quantum) {
                    time+=quantum;
                    remBT[i]-=quantum;
                } else {
                    time+=remBT[i];
                    wt[i]=time-p[i].bt-p[i].at;
                    if (wt[i]<0) wt[i]=0;
                    tat[i]=wt[i]+p[i].bt;
                    remBT[i]=0;
                    completed++;
                }
            }
        }
        if (doneCycle) time++; // CPU idle
    }

    printf("\n--- Round Robin Scheduling (q=%d) ---\n", quantum);
    printf("PID\tAT\tBT\tWT\tTAT\n");
    for (int i=0;i<n;i++) {
        printf("P%d\t%d\t%d\t%d\t%d\n",p[i].pid,p[i].at,p[i].bt,wt[i],tat[i]);
        totalWT+=wt[i]; totalTAT+=tat[i];
    }
    printf("Average WT = %.2f\n", (float)totalWT/n);
    printf("Average TAT = %.2f\n", (float)totalTAT/n);
}

// ---------- MAIN ----------
int main() {
    int n, q;
    printf("Enter number of processes: ");
    scanf("%d",&n);
    struct Process p[n];

    printf("Enter process details (AT BT PRI):\n");
    for (int i=0;i<n;i++) {
        p[i].pid=i+1;
        scanf("%d %d %d",&p[i].at,&p[i].bt,&p[i].pri);
    }

    printf("Enter time quantum for RR: ");
    scanf("%d",&q);

    FCFS(p,n);
    SJF(p,n);
    PriorityScheduling(p,n);
    RoundRobin(p,n,q);

    return 0;
}
