#include "rbtree.h"
#include <stdio.h>
#include <stdlib.h>

void rbtree_insert_fixup(rbtree *t, node_t *node);
void left_rotate(rbtree *t, node_t *cur_node);
void right_rotate(rbtree *t, node_t *cur_node);
void rb_transplant(rbtree *t, node_t *change_node, node_t *new_node);
node_t *tree_minimum(rbtree *t, node_t *cur_node);
void rb_delete_fixup(rbtree *t, node_t *child_node);

rbtree *new_rbtree(void)
{ // rb tree 구조체 생성
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  // nil_node 생성, calloc을 사용하여 모든 node값에 null(=0)로 초기화 해줄 수 있다. 포인터 변수는 0으로 초기화하면 null 포인터를 나타낸다.
  node_t *nil_node = (node_t *)calloc(1, sizeof(node_t)); // 1 -> 할당하고자 하는 객체의 개수 , sizeof(rbtree)-> 각 객체의 크기
  // nil_node는 모두 black
  nil_node->color = 1; // 1 -> rbtree_black을 뜻함, nil-node는 모두 black

  p->nil = nil_node;  // nilnode 붙여주기
  p->root = nil_node; // tree가 비어있을 경우 root는 nil
  nil_node->left = nil_node;
  nil_node->right = nil_node; // nil의 left, right를 nil로 처리
  return p;
}

void delete_rbtree_node(rbtree *t, node_t *node)
{ // 노드 삭제
  if (node == t->nil)
  {
    return;
  }
  delete_rbtree_node(t, node->left); // 재귀
  delete_rbtree_node(t, node->right);
  free(node); // 해당 node 할당 해제
}

void delete_rbtree(rbtree *t) // rb tree 구조체가 차지했던 메모리 반환
{
  if (t == NULL)
  {
    return;
  }
  delete_rbtree_node(t, t->root); // root 노드 부터 차례대로 할당 해제
  free(t->nil);                   // nil 노드 할당 해제
  free(t);                        // rb tree 할당 해제
}

node_t *rbtree_insert(rbtree *t, const key_t key)
{ // key 추가 (rb tree 삽입)
  node_t *new_node = (node_t *)calloc(1, sizeof(node_t));
  node_t *pre_node = t->nil;  // 새로 삽일 할 노드가 들어갈 위치에 부모노드를 저장
  node_t *cur_node = t->root; // 새로 삽입할 key값을 넣을 노드의 위치 저장
  new_node->key = key;        // 삽입할 노드에 키 값 설정
  while (cur_node != t->nil)
  {
    pre_node = cur_node; // 현재노드를 pre노드에 저장
    if (new_node->key < cur_node->key)
    { // 삽입할 key값이 현재 노드보다 작을 경우 현재 노드에서 left로
      cur_node = cur_node->left;
    }
    else
    {
      cur_node = cur_node->right; // 삽일할 key값이 현재 노드보다 클 경우 현재 노드에서 right로
    }
  }
  new_node->parent = pre_node; // 새로운 노드의 부모노드 저장
  // 현재 노드의 부모노드로 이전 노드 연결
  if (pre_node == t->nil)
  { // 트리에 노드가 없을 경우 , 삽입할 노드가 root 노드가 된다.
    t->root = new_node;
  }
  else if (key < pre_node->key)
  {                            // 이전 노드의 key값이 현재 노드보다 클 경우
    pre_node->left = new_node; // 현재 노드를 이전 노드의 왼쪽에 위치하게 한다.
  }
  else
  {
    pre_node->right = new_node; // 현재 노드를 이전 노드의 오른쪽에 위치하게 한다.
  }
  // 이전 노드의 자녀노드로 현재노드를 연결
  new_node->left = t->nil; // 올바른 트리 구조를 유지하기 위해 삽입된 노드의 위치는 leaf-node에 위치하며, 자녀노드로 nil_node를 가진다.
  new_node->right = t->nil;
  new_node->color = 0; // 삽인된 노드의 컬러는 red
  rbtree_insert_fixup(t, new_node);
  return t->root;
}

void rbtree_insert_fixup(rbtree *t, node_t *cur_node)
{                // 삽입 이후 rb tree 속성을 만족시키기 위한 재조정
  node_t *uncle; // cur_node의 부모의 형제 노드를 저장
  while (cur_node->parent->color == 0)
  { // cur_node의 부모가 red일 경우 (red가 아닐때까지 while문 반복)
    if (cur_node->parent == cur_node->parent->parent->left)
    {                                          // cur_node가 left쪽에 있을 경우
      uncle = cur_node->parent->parent->right; // uncle은 cur_node 부모의 형제 노드
      if (uncle->color == 0)
      {                                      // cur_node의 부모와 uncle 모두 red일 경우 (case 3)
        cur_node->parent->color = 1;         // cur_node의 부모의 색을 black으로  바꿈
        uncle->color = 1;                    // cur_node의 uncle의 색을 black으로 바꿈
        cur_node->parent->parent->color = 0; // cur_node의 부모의 부모의 색 red로 바꿈
        cur_node = cur_node->parent->parent; // cur_node 와 cur_node의 부모의 부모로 바꿔주고 다시 속성이 맞는지 확인
      }
      else
      { // cur_node의 부모 노드만 red일 경우 (uncle은 black) -> LR 케이스
        if (cur_node == cur_node->parent->right)
        {                              // cur_node 가 부모의 오른쪽 자식일 경우
          cur_node = cur_node->parent; // cur_node의 기준을 cur_node의 부모로 바꾼후 left rotate
          left_rotate(t, cur_node);
        } // cur_node의 부모 노드만 red이고, cur_node가 부모의 왼쪽 자식일 경우  -> LL 케이스 (LR 케이스를 수정하면 LL케이스가 된다.)
        cur_node->parent->color = 1;               // cur_node의 부모의 색을 black으로 바꿈
        cur_node->parent->parent->color = 0;       // cur_node의 부모의 부모의 색을 red로 바꿈
        right_rotate(t, cur_node->parent->parent); // 오른쪽 회전 기준을 cur_node의 부모의 부모로 바꿈
      }
    }
    else
    { // cur_node가 right쪽에 있을 경우 // 위와 방향만 바꾸고 동일
      uncle = cur_node->parent->parent->left;
      if (uncle->color == 0)
      { // cur_node의 부모와 uncle 모두 red일 경우 (case 3)
        cur_node->parent->color = 1;
        uncle->color = 1;
        cur_node->parent->parent->color = 0;
        cur_node = cur_node->parent->parent;
      }
      else
      { // cur_node의 부모 노드만 red일 경우 (uncle은 black) -> RL 케이스
        if (cur_node == cur_node->parent->left)
        {
          cur_node = cur_node->parent;
          right_rotate(t, cur_node);
        } // cur_node의 부모 노드만 red이고, cur_node가 부모의 오른쪽 자식일 경우  -> RR 케이스 (RL 케이스를 수정하면 RR케이스가 된다.)
        cur_node->parent->color = 1;
        cur_node->parent->parent->color = 0;
        left_rotate(t, cur_node->parent->parent);
      }
    }
  }
  t->root->color = 1;
}

void left_rotate(rbtree *t, node_t *cur_node)
{                                       // cur_node를 기준으로 왼쪽으로 회전
  node_t *new_parent = cur_node->right; //  new_parent를 curnode의 오른쪽 자녀 노드로 설정
  cur_node->right = new_parent->left;   //  cur_node의 오른쪽 자녀에 new_parent의 왼쪽 자녀 붙이기
  if (new_parent->left != t->nil)
  {                                      //  new_parent의 왼쪽 자녀가 nil이 아닐때만  -> nil 노드에는 부모를 설정하면 안되기 때문
    new_parent->left->parent = cur_node; // new_parent의 왼쪽자녀의 부모를 cur_node로 설정
  }
  new_parent->parent = cur_node->parent; // cur_node가 가지고 있던 부모를 new_parent의 부모로 설정
  if (cur_node->parent == t->nil)
  { // cur_node의 부모가 null인 경우 new_parent가 root 노드가 됨
    t->root = new_parent;
  }
  else if (cur_node == cur_node->parent->left)
  {                                      // parent의 왼쪽 자녀가 cur_node일 경우
    cur_node->parent->left = new_parent; // parent의 왼쪽 자녀노드를 new_parent로 변경
  }
  else
  {
    cur_node->parent->right = new_parent; // parent의 오른쪽 자녀가 cur_node일 경우
  } // parent의 오른쪽 자녀를 new_parent로 변경
  new_parent->left = cur_node;
  cur_node->parent = new_parent; // cur_node의 부모를 new_parent로, new_parent의 왼쪽 자녀를 cur_node로 설정
}

void right_rotate(rbtree *t, node_t *cur_node)
{ // cur_node를 기준으로 왼쪽으로 회전

  node_t *new_parent = cur_node->left; //  new_parent를 curnode의 왼쪽 자녀 노드로 설정
  cur_node->left = new_parent->right;  //  cur_node의 왼쪽 자녀에 new_parent의 오른쪽 자녀 붙이기
  if (new_parent->right != t->nil)
  {                                       //  new_parent의 오른쪽 자녀가 nil이 아닐때만  -> nil 노드에는 부모를 설정하면 안되기 때문
    new_parent->right->parent = cur_node; // new_parent의 오른쪽자녀의 부모를 cur_node로 설정
  }
  new_parent->parent = cur_node->parent; // cur_node가 가지고 있던 부모를 new_parent의 부모로 설정
  if (cur_node->parent == t->nil)
  { // cur_node의 부모가 null인 경우 new_parent가 root 노드가 됨
    t->root = new_parent;
  }
  else if (cur_node == cur_node->parent->right)
  {                                       // parent의 오른쪽 자녀가 cur_node일 경우
    cur_node->parent->right = new_parent; // parent의 오른쪽 자녀노드를 new_parent로 변경
  }
  else
  {
    cur_node->parent->left = new_parent; // parent의 왼쪽 자녀가 cur_node일 경우
  } // parent의 왼쪽 자녀를 new_parent로 변경
  new_parent->right = cur_node;
  cur_node->parent = new_parent; // cur_node의 부모를 new_parent로, new_parent의 오른쪽 자녀를 cur_node로 설정
}

node_t *rbtree_find(const rbtree *t, const key_t key)
{ // rb tree 내에 해당 key가 있는지 탐색하여 있으면 해당 node pointer 반환
  node_t *node = t->root;
  while (node != t->nil)
  {
    if (node->key == key)
    {
      return node; // 주어진 key값을 찾으면 해당 노드 반환
    }
    else if (node->key > key) // 주어진 key 값이 더 작을 경우 left 탐색
    {
      node = node->left;
    }
    else if (node->key < key) // 주어진 key 값이 더 클 경우 right 탐색
    {
      node = node->right;
    }
  }
  return NULL; // key값이 없을 경우 null 반환
}

node_t *rbtree_min(const rbtree *t)
{ // rb tree 중 최소 값을 가진 node pointer 반환

  node_t *node = t->root;
  if (node == NULL)
  {
    return NULL;
  }
  while (node->left != t->nil)
  {
    node = node->left; // 가장 왼쪽에 있는 노드를 반환 (nil을 만나기 전까지 계속 left)
  }
  return node;
}

node_t *rbtree_max(const rbtree *t)
{ // rb tree 중 최대 값을 가진 node pointer 반환
  node_t *node = t->root;
  if (node == NULL)
  {
    return NULL;
  }
  while (node->right != t->nil)
  {
    node = node->right; // 가장 오른쪽에 있는 노드를 반환 (nil을 만나기 전까지 계속 right)
  }
  return node;
}

int rbtree_erase(rbtree *t, node_t *p) // ptr로 지정된 node를 삭제하고 메모리 반환
{
  node_t *change_node = p;         // 전임자를 받을 노드
  color_t save_color;              // 삭제되는 색을 저장
  node_t *child_node;              // 전임자 노드의 자녀 노드
  save_color = change_node->color; // 여기선 삭제될 노드의 색을 save_color에 저장 (삭제되는 노드의 색 = 삭제되는 색)
  if (p->left == t->nil)           // 삭제될 노드의 자녀가 오른쪽에 하나 있을 경우 (두 자녀 모두 nil일 경우 nil노드가 삭제될 노드의 부모노드에 자녀노드가 된다. )
  {
    child_node = p->right;         // 삭제될 노드의 오른쪽 자녀노드 -> child_node
    rb_transplant(t, p, p->right); // 삭제될 노드(p)의 오른쪽자녀 노드가 삭제될 노드의 자리를 대체한다.
  }
  else if (p->right == t->nil) // 삭제될 노드의 왼쪽 자녀노드 -> child_node
  {
    child_node = p->left;
    rb_transplant(t, p, p->left);
  }
  else // 삭제될 노드의 자녀가 두개 이상일 경우
  {
    change_node = tree_minimum(t, p->right); // minimum함수를 통해 전임자(change_node)를 찾는다.
    save_color = change_node->color;         // 전임자의 색을 save_color에 저장한다.
    child_node = change_node->right;         // 전임자 자리에 있는 노드 -> change_node, change_node의 오른쪽 노드 -> child 노드
    if (change_node->parent == p)            // 삭제되는 노드의 전임자가 삭제되는 노드의 오른쪽 자녀노드 일때
    {
      child_node->parent = change_node;
    }
    else
    {
      rb_transplant(t, change_node, change_node->right); // 전임자에 위치한 노드가 사라지므로 전임자의 오른쪽 자녀를 해당 자리로 위치시킨다.
      change_node->right = p->right;                     // 삭제된 노드의 오른쪽 자녀노드와 전임자 노드 연결
      change_node->right->parent = change_node;
    }
    rb_transplant(t, p, change_node); // 삭제되는 노드자리에 전임자를 대체한다.
    change_node->left = p->left;
    change_node->left->parent = change_node;
    change_node->color = p->color; // 전임자 노드의 색을 삭제되는 노드의 색을 가진다.
  }
  if (save_color == 1) // 없어진 노드 자리의 color가 black일 경우 레드블랙트리의 속성을 위배할 수 있음으로 fixup 함수를 호출
  {
    rb_delete_fixup(t, child_node); // child노드는 삭제된 노드의 자리르 대체하므로 doubly black이 붙는 노드이다. doubly black을 기준으로 fixup함수를 실행한다.
  }
  free(p);
  return 0;
}

void rb_delete_fixup(rbtree *t, node_t *child_node)
{ // case에 따라 doubly black을 해결하는 함수
  while (child_node != t->root && child_node->color == 1)
  { // childe_node -> doubly black이 붙어있는 상태라고 생각
    if (child_node == child_node->parent->left)
    {                                                   // child node가 부모 노도의 왼쪽에 위치할 경우
      node_t *brother_node = child_node->parent->right; // brother 노드는 child node의 형제 노드
      if (brother_node->color == 0)                     // 형제노드의 color가 red일 경우  -> case1
      {
        brother_node->color = 1;
        child_node->parent->color = 0;
        left_rotate(t, child_node->parent);
        brother_node = child_node->parent->right;
      }
      if (brother_node->left->color == 1 && brother_node->right->color == 1) // 형제 노드가 black이면서 형제 노드의 자녀노드가 모두 black일 경우 -> case2
      {
        brother_node->color = 0;
        child_node = child_node->parent;
      }
      else
      {
        if (brother_node->right->color == 1)
        { // 형제 노드의 오른쪽 자녀 노드가 black, 왼쪽 자녀 노드가 red인 경우 -> case3
          brother_node->left->color = 1;
          brother_node->color = 0;
          right_rotate(t, brother_node);
          brother_node = child_node->parent->right;
        } // 형제 노드의 왼쪽 자녀 노드만 red인경우 -> case4
        brother_node->color = child_node->parent->color;
        child_node->parent->color = 1;
        brother_node->right->color = 1;
        left_rotate(t, child_node->parent);
        child_node = t->root;
      }
    }
    else // child node가 부모 노도의 오른쪽에 위치할 경우
    {
      node_t *brother_node = child_node->parent->left;
      if (brother_node->color == 0) // 형제노드의 color가 red일 경우  -> case1
      {
        brother_node->color = 1;
        child_node->parent->color = 0;
        right_rotate(t, child_node->parent);
        brother_node = child_node->parent->left;
      }
      if (brother_node->right->color == 1 && brother_node->left->color == 1) // 형제 노드가 black이면서 형제 노드의 자녀노드가 모두 black일 경우 -> case2
      {
        brother_node->color = 0;
        child_node = child_node->parent;
      }
      else
      {
        if (brother_node->left->color == 1) // 형제 노드의 오른쪽 자녀 노드가 black, 왼쪽 자녀 노드가 red인 경우 -> case3
        {
          brother_node->right->color = 1;
          brother_node->color = 0;
          left_rotate(t, brother_node);
          brother_node = child_node->parent->left;
        } // 형제 노드의 왼쪽 자녀 노드만 red인경우 -> case4
        brother_node->color = child_node->parent->color;
        child_node->parent->color = 1;
        brother_node->left->color = 1;
        right_rotate(t, child_node->parent);
        child_node = t->root;
      }
    }
  }
  child_node->color = 1;
}

void rb_transplant(rbtree *t, node_t *change_node, node_t *new_node)
{                                    // change 노드를 대신해서 new 노드를 그 자리에 넣는 함수
  if (change_node->parent == t->nil) // 대체되는 노드의 부모가 없을 경우
  {
    t->root = new_node; // 삭제한 노드이 자녀가 부모가 노드가 된다.
  }
  else if (change_node == change_node->parent->left) // 대체되는 노드가 부모노드의 왼쪽에 위치할 경우
  {
    change_node->parent->left = new_node; // 대체되는 노드의 자녀노드를 부모노드의 왼쪽에 연결
  }
  else
  {                                        // 대체되는 노드가 부모노드의 오른쪽에 위치할 경우
    change_node->parent->right = new_node; // 대체되는 노드의 자녀노드를 부모노드의 오른쪽에 연결
  }
  new_node->parent = change_node->parent; // 새로운노드의 부모노드를 대체되는 노드의 부모노드로 바꿔준다.
}

node_t *tree_minimum(rbtree *t, node_t *cur_node) // 삭제 과정에서 삭제노드의 자녀노드가 두개일때 successor(후임자)로 대체
{
  while (1)
  {
    if (cur_node->left == t->nil) // cur_node의 왼쪽 자녀노드가 nil일 경우 cur_node 반환
    {
      return cur_node;
    }
    cur_node = cur_node->left;
  }
}

void array_traversal(const rbtree *t, node_t *node, key_t *arr, int *index, const size_t n)   // rb tree 순회 함수  
{
  if (node == t->nil || *index >= n) // node가 nil이거나 index값이 배열의 크기를 넘어갈 경우 return
  {
    return;
  }

  array_traversal(t, node->left, arr, index, n);
  arr[*index] = node->key; // arr에 해당 인덱스에 node의 key값을 넣음
  (*index)++;              // 각 재귀호출간에 index값을 공유하도록 int 포인터 사용
  array_traversal(t, node->right, arr, index, n);
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) // rb tree의 내용을 key순서대로 주어진 array 반환
{
  int index = 0;                               // 배열의 index값 지정
  array_traversal(t, t->root, arr, &index, n); // 주소값을 넘겨주어서 index값을 공유할 수 있도록 한다.

  return 0;
}
