#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

static int WP_NO;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(char *args){
	WP* p = free_;
	free_ = free_->next;
	if(free_ == NULL){
		assert(0);
	}
	p->next = NULL;
	
	strcpy(p->expr, args);
	bool *success = false;
	p->old_val = expr(args, success);
	
	if(head == NULL){
		head = p;
		WP_NO = 0;
		p->NO = WP_NO;
	}
	else{
		WP_NO++;
		WP* q = head;
		while(q->next != NULL){
			q = q->next;
		}
		q->next = p;
		p->NO = WP_NO;
	}
	return p;
}

void free_wp(WP *wp){
	if(wp == NULL){
		assert(0);
	}
	if(wp == head){
		head = head->next;
	}
	else{
		WP* p = head;
		while(p->next != wp){
			p = p->next;
		}
		p->next = wp->next;
	}
	wp->next = free_;
	free_ = wp;
	wp->new_val = 0;
	wp->expr[0] = '\0';
}

void list_watchpoint(){
	WP* p = head;
	printf("NO\tExpr\tOld Value\t\n");
	while(p!=NULL){
		printf("%d\t%s\t%d\n", p->NO, p->expr, p->old_val);
		p = p->next;
	}
}


int set_watchpoint(char *e){
	WP* p = new_wp(e);
	return p->NO;
	return 0;
}

bool delete_watchpoint(int NO){
	WP* p = head;
	while(p!=NULL && p->NO!=NO){
		p = p->next;
	}
	if(p==NULL){
		printf("Not found\n");
		return false;
	}
	else{
		printf("Watchpoint %d deleted\n", p->NO);
		free_wp(p);
		return true;
	}
}

WP* scan_watchpoint(){
	WP* p = head;
	int num;
	bool* success=false;

	while(p!=NULL){
		num = expr(p->expr, success);
		if(num != p->old_val){

			p->new_val = num;
			return p;
		}
		p = p->next;
	}
	return p;
}