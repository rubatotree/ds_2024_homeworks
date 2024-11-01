#include <iostream>
#include <cstdio>
#include <tuple>
#include <algorithm>
#include <functional>

template <class T>
struct SegmentTreeNode
{
	int arg;
	SegmentTreeNode *parent, *lchild, *rchild;
};
template <class T>
SegmentTreeNode<T>* build_seg_tree(SegmentTreeNode<T> *p, std::vector<T> &arr, int l, int r, std::function<bool(T&, T&)> cmp)
{
	if(r == l + 1) // Leaf
	{
		p->arg = l;
		return p;
	}
	p->lchild = new SegmentTreeNode<T>();
	p->rchild = new SegmentTreeNode<T>();
	p->lchild->parent = p->rchild->parent = p;
	int mid = (l + r + 1) >> 1;
	auto lmin = build_seg_tree(p->lchild, arr, l, mid, cmp);
	auto rmin = build_seg_tree(p->rchild, arr, mid, r, cmp);
	bool cmp_res = cmp(arr[p->lchild->arg], arr[p->rchild->arg]);
	p->arg = cmp_res ? p->lchild->arg : p->rchild->arg;
	return cmp_res ? lmin : rmin;
}
template <class T>
void del_seg_tree(SegmentTreeNode<T> *p)
{
	if(p == NULL) return;
	del_seg_tree(p->lchild);
	del_seg_tree(p->rchild);
	delete p;
}
template <class T>
std::pair<int, int> find_two_mins(std::vector<T> &arr, std::function<bool(T&, T&)> cmp)
{
	int n = arr.size();
	if(n <= 1) return { 0, 0 };
	auto seg_root = new SegmentTreeNode<T>();
	seg_root->parent = nullptr;
	auto min_ptr = build_seg_tree<T>(seg_root, arr, 0, n, cmp);
	int min1 = seg_root->arg, min2 = -1, arg;
	auto p = min_ptr;
	if(p->parent == nullptr) return { min1, min1 };
	while(p->parent != nullptr)
	{
		if(p == p->parent->lchild) arg = p->parent->rchild->arg;
		else arg = p->parent->lchild->arg;
		p = p->parent;
		if(min2 == -1) min2 = arg;
		else
		{
			bool cmp_res = cmp(arr[arg], arr[min2]);
			if(cmp_res) min2 = arg;
		}
	}
	del_seg_tree(seg_root);
	return { min1, min2 };
};

int main()
{
	std::vector<int> arr;
	int n;
	scanf("%d", &n);
	for(int i = 0; i < n; i++)
	{
		int x;
		scanf("%d", &x);
		arr.push_back(x);
	}
	auto two_mins = find_two_mins<int>(arr, [arr](int &a, int &b) -> bool { return a < b; });
	printf("%d %d\n", two_mins.first, two_mins.second);
	return 0;
}
