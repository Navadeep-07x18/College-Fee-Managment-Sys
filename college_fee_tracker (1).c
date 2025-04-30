
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define FILE_NAME "records.txt"
#define BACKUP_FILE "backup_records.txt"
#define ADMIN_PASSWORD "admin123"
#define MAX_LINE 300

typedef struct {
    char student_id[20];
    char name[100];
    char course[50];
    int year;
    float amount;
    char timestamp[100];
} Payment;

// Function Prototypes
int admin_login();
void record_payment();
void print_receipt();
void display_records();
void search_by_id();
void summary_report();
void delete_record();
void clear_records();
void export_csv();
void backup_records();
void pause();
int parse_payment_line(const char *line, Payment *p);
int is_duplicate_payment(Payment *new_p);

void get_input(char *buffer, int size) {
    fgets(buffer, size, stdin);
    buffer[strcspn(buffer, "\n")] = 0;
}

int parse_payment_line(const char *line, Payment *p) {
    return sscanf(line, "%19[^,],%99[^,],%49[^,],%d,%f,%99[^\n]",
                  p->student_id, p->name, p->course, &p->year, &p->amount, p->timestamp) == 6;
}

int admin_login() {
    char password[50];
    printf("\nEnter Admin Password: ");
    get_input(password, sizeof(password));
    if (strcmp(password, ADMIN_PASSWORD) == 0) {
        printf("Login successful.\n");
        return 1;
    } else {
        printf("Incorrect password. Access denied.\n");
        return 0;
    }
}

int is_duplicate_payment(Payment *new_p) {
    FILE *fp = fopen(FILE_NAME, "r");
    char line[MAX_LINE];
    if (!fp) return 0;

    while (fgets(line, sizeof(line), fp)) {
        Payment p;
        if (parse_payment_line(line, &p)) {
            if (strcmp(p.student_id, new_p->student_id) == 0 &&
                p.amount == new_p->amount &&
                strcmp(p.course, new_p->course) == 0 &&
                p.year == new_p->year) {
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

void record_payment() {
    Payment payment;
    FILE *fp = fopen(FILE_NAME, "a");
    if (!fp) {
        printf("Error opening file!\n");
        return;
    }

    printf("\nEnter Student ID: ");
    get_input(payment.student_id, sizeof(payment.student_id));
    if (strlen(payment.student_id) == 0) {
        printf("Student ID cannot be empty.\n");
        fclose(fp);
        return;
    }

    printf("Enter Student Name: ");
    get_input(payment.name, sizeof(payment.name));
    if (strlen(payment.name) == 0) {
        printf("Student name cannot be empty.\n");
        fclose(fp);
        return;
    }

    printf("Enter Course: ");
    get_input(payment.course, sizeof(payment.course));

    printf("Enter Year (1-4): ");
    if (scanf("%d", &payment.year) != 1 || payment.year < 1 || payment.year > 4) {
        printf("Invalid year.\n");
        while (getchar() != '\n');
        fclose(fp);
        return;
    }
    while (getchar() != '\n');

    printf("Enter Amount Paid: ");
    if (scanf("%f", &payment.amount) != 1 || payment.amount <= 0) {
        printf("Invalid amount.\n");
        while (getchar() != '\n');
        fclose(fp);
        return;
    }
    while (getchar() != '\n');

    time_t now = time(NULL);
    strftime(payment.timestamp, sizeof(payment.timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    if (is_duplicate_payment(&payment)) {
        printf("Duplicate payment detected. Entry skipped.\n");
        fclose(fp);
        return;
    }

    fprintf(fp, "%s,%s,%s,%d,%.2f,%s\n",
            payment.student_id, payment.name, payment.course, payment.year, payment.amount, payment.timestamp);
    fclose(fp);
    printf("Payment recorded successfully.\n");
}

void print_receipt() {
    char student_id[20];
    char line[MAX_LINE];
    int found = 0;

    printf("\nEnter Student ID to print receipt: ");
    get_input(student_id, sizeof(student_id));

    FILE *fp = fopen(FILE_NAME, "r");
    if (!fp) {
        printf("No records found.\n");
        return;
    }

    printf("\n----- Receipts for ID: %s -----\n", student_id);
    while (fgets(line, sizeof(line), fp)) {
        Payment p;
        if (parse_payment_line(line, &p) && strcmp(p.student_id, student_id) == 0) {
            printf("Student Name: %s\nCourse: %s\nYear: %d\nAmount: ₹%.2f\nDate/Time: %s\n---------------------------\n",
                   p.name, p.course, p.year, p.amount, p.timestamp);
            found = 1;
        }
    }
    if (!found)
        printf("No payments found for student ID %s.\n", student_id);
    fclose(fp);
}

void display_records() {
    FILE *fp = fopen(FILE_NAME, "r");
    char line[MAX_LINE];
    if (!fp) {
        printf("No records found.\n");
        return;
    }

    printf("\n--- All Payment Records ---\n");
    while (fgets(line, sizeof(line), fp)) {
        Payment p;
        if (parse_payment_line(line, &p)) {
            printf("ID: %-10s Name: %-20s ₹%.2f [%s]\n",
                   p.student_id, p.name, p.amount, p.timestamp);
        }
    }
    fclose(fp);
}

void search_by_id() {
    print_receipt();
}

void summary_report() {
    FILE *fp = fopen(FILE_NAME, "r");
    char line[MAX_LINE];
    int count = 0;
    float total = 0, max = 0, min = 999999;

    if (!fp) {
        printf("No records found.\n");
        return;
    }

    printf("\n--- Summary Report ---\n");

    while (fgets(line, sizeof(line), fp)) {
        Payment p;
        if (parse_payment_line(line, &p)) {
            total += p.amount;
            if (p.amount > max) max = p.amount;
            if (p.amount < min) min = p.amount;
            count++;
        }
    }
    fclose(fp);

    printf("Total Payments: %d\n", count);
    printf("Total Amount Collected: ₹%.2f\n", total);
    if (count > 0) {
        printf("Average Payment: ₹%.2f\n", total / count);
        printf("Highest Payment: ₹%.2f\n", max);
        printf("Lowest Payment: ₹%.2f\n", min);
    }
}

void delete_record() {
    char student_id[20];
    char line[MAX_LINE];
    Payment matches[100];
    int match_count = 0;

    printf("\nEnter Student ID of record to delete: ");
    get_input(student_id, sizeof(student_id));

    FILE *fp = fopen(FILE_NAME, "r");
    if (!fp) {
        printf("Error opening file.\n");
        return;
    }

    while (fgets(line, sizeof(line), fp)) {
        Payment p;
        if (parse_payment_line(line, &p) && strcmp(p.student_id, student_id) == 0) {
            matches[match_count++] = p;
        }
    }
    fclose(fp);

    if (match_count == 0) {
        printf("No matching records found.\n");
        return;
    }

    printf("\nMatching Records:\n");
    for (int i = 0; i < match_count; ++i) {
        printf("%d. ₹%.2f on %s (Course: %s, Year: %d)\n",
               i + 1, matches[i].amount, matches[i].timestamp, matches[i].course, matches[i].year);
    }

    int choice;
    printf("\nEnter the number of the record to delete (0 to cancel): ");
    if (scanf("%d", &choice) != 1 || choice < 0 || choice > match_count) {
        while (getchar() != '\n');
        printf("Invalid input.\n");
        return;
    }
    while (getchar() != '\n');

    if (choice == 0) {
        printf("Deletion canceled.\n");
        return;
    }

    backup_records();

    FILE *in = fopen(FILE_NAME, "r");
    FILE *out = fopen("temp.txt", "w");
    if (!in || !out) {
        printf("Error accessing files.\n");
        return;
    }

    while (fgets(line, sizeof(line), in)) {
        Payment p;
        if (parse_payment_line(line, &p) &&
            strcmp(p.student_id, matches[choice - 1].student_id) == 0 &&
            strcmp(p.timestamp, matches[choice - 1].timestamp) == 0) {
            continue;
        }
        fputs(line, out);
    }

    fclose(in);
    fclose(out);
    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);
    printf("Record deleted successfully.\n");
}

void clear_records() {
    char confirm[5];
    printf("Are you sure you want to clear all records? Type 'yes' to confirm: ");
    get_input(confirm, sizeof(confirm));
    if (strcmp(confirm, "yes") != 0) {
        printf("Operation canceled.\n");
        return;
    }

    backup_records();
    if (remove(FILE_NAME) == 0)
        printf("All records cleared.\n");
    else
        printf("No records to clear or error deleting file.\n");
}

void export_csv() {
    FILE *in = fopen(FILE_NAME, "r");
    FILE *out = fopen("records_export.csv", "w");
    char line[MAX_LINE];
    if (!in || !out) {
        printf("Error exporting data.\n");
        return;
    }
    fprintf(out, "StudentID,Name,Course,Year,Amount,Timestamp\n");
    while (fgets(line, sizeof(line), in)) {
        fputs(line, out);
    }
    fclose(in);
    fclose(out);
    printf("Exported to records_export.csv successfully.\n");
}

void backup_records() {
    FILE *src = fopen(FILE_NAME, "r");
    FILE *dst = fopen(BACKUP_FILE, "w");
    char ch;
    if (!src || !dst) {
        printf("Backup failed.\n");
        return;
    }
    while ((ch = fgetc(src)) != EOF) fputc(ch, dst);
    fclose(src);
    fclose(dst);
}

void pause() {
    printf("\nPress Enter to continue...");
    getchar();
}

int main() {
    if (!admin_login()) return 0;

    int choice;
    do {
        printf("\n=== College Fee Payment Tracker ===\n");
        printf("1. Record Payment\n");
        printf("2. Print Receipt\n");
        printf("3. Show All Records\n");
        printf("4. Search by Student ID\n");
        printf("5. Summary Report\n");
        printf("6. Delete Record\n");
        printf("7. Clear All Records\n");
        printf("8. Export to CSV\n");
        printf("9. Exit\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        while (getchar() != '\n');

        switch (choice) {
            case 1: record_payment(); break;
            case 2: print_receipt(); break;
            case 3: display_records(); break;
            case 4: search_by_id(); break;
            case 5: summary_report(); break;
            case 6: delete_record(); break;
            case 7: clear_records(); break;
            case 8: export_csv(); break;
            case 9: backup_records(); printf("Exiting... Backup saved.\n"); break;
            default: printf("Invalid choice.\n"); break;
        }
        if (choice != 9) pause();
    } while (choice != 9);

    return 0;
}
