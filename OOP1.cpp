#include<bits/stdc++.h>
using namespace std;

map<int,int> ans[1030];
int node_num, dst_num, link_num;
class dat{
    public:
    vector<pair<int, int> > arr;
}link_old[100000],link_new[100000];

void dijkstra(int dst, dat link[100000]){
    map<int, int> pre, dis, flag;
    priority_queue< pair<int, int>, vector< pair<int,int> >, greater< pair<int,int> > > qu;
    for(int i=0; i<node_num; i++){
        dis[i] = 0x3f3f3f3f;
        pre[i]=-1;
        flag[i]=0;
    }

    dis[dst] = 0;
    pre[dst] = dst;
    qu.push(make_pair(0,dst)); //dis nodeID

    while(!qu.empty()){
        int key=qu.top().second, len = link[key].arr.size();
        qu.pop();
        if(flag[key]!=1){
            flag[key]=1;
            for(int i=0; i<len; i++){
                int neighbor = link[key].arr[i].second, dis_neighbor = link[key].arr[i].first;
                if(flag[neighbor]!=1){
                    if(dis[key]+dis_neighbor < dis[neighbor]){
                        dis[neighbor] = dis[key]+dis_neighbor;    
                        pre[neighbor]=key;
                    }
                    else if(dis[key]+dis_neighbor == dis[neighbor]){
                        pre[neighbor]=min(key,pre[neighbor]);
                    }
                    qu.push(make_pair(dis[neighbor],neighbor));
                }
            }
        }
    }
    for(int i=0; i<node_num; i++){
        ans[i].insert({dst,pre[i]});
    }
}

int main(){

    cin >> node_num >> dst_num >> link_num;

    int dst[dst_num+5];

    for(int i=0; i<dst_num; i++){
        cin >> dst[i];
    }

    for(int i=0; i<link_num; i++){
        int linkID, Node1, Node2, oldW, newW;
        cin >> linkID >> Node1 >> Node2 >> oldW >> newW;
        link_old[Node1].arr.push_back(make_pair(oldW, Node2));
        link_old[Node2].arr.push_back(make_pair(oldW, Node1));
        link_new[Node1].arr.push_back(make_pair(newW, Node2));
        link_new[Node2].arr.push_back(make_pair(newW, Node1));
    }

    for(int i=0; i<dst_num; i++){
        dijkstra(dst[i], link_old);
    }

    for(int i=0; i<dst_num; i++){
        dijkstra(dst[i], link_new);
    }    
    for(int i=0; i<node_num; i++){
        cout << i << "\n";
        for(int j=0; j<dst_num; j++){
            if(dst[j]!=i){
                cout << dst[j] << " " << ans[i][j] << "\n"; 
            }
        }
    }

    int temp = 2*dst_num;

    for(int i=0; i<node_num; i++){
        int ans_flag=0;
        for(int j=dst_num; j<temp; j++){
            if(ans[i][j-dst_num] != ans[i][j]){
                if(ans_flag!=1){
                    cout << i << "\n";
                    ans_flag=1;
                }
                cout << dst[j-dst_num] << " " << ans[i][j] << "\n";
            }
        }
    }
    return 0;
}