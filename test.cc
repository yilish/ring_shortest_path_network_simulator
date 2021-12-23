#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

// #include "../rawAlgos/MultiRingNet.h"
#include <vector>
#include <list>
using namespace std;
using namespace ns3;


NS_LOG_COMPONENT_DEFINE("TestExample");

class MultiRingNet
{
public:
	MultiRingNet(int r, int rn, int cn): ringNum(r), ringNodeNum(rn), connectedNodeNum(cn) { }
	vector<int> dijkstra(int start, int end) {
        vector<int> dis(graph.size());
        vector<int> parent(graph.size());
        dis[start] = 0;
        for(int i = 0; i < int(graph.size()); i++) {
            if (i != start) {
                dis[i] = 2e9;
                parent[i] = -1;
            }
        }
        
        vector<int> visited(graph.size(), 0);
        priority_queue<pair<int, int>> q;
        q.push({dis[start], start});
        while (q.size()) {
            // NS_LOG_UNCOND(q.top());

            auto minNode = q.top();
            q.pop();

            int u = minNode.second;
            if (visited[u]) {
                continue;
            }
            visited[u] = true;
            for (auto v : graph[u]) {
            
                if (v != u && !visited[v] &&  dis[v] > dis[u] + 1) {
                    dis[v] = dis[u] + 1;
                    parent[v] = u;
                    // if (! visited[v]) {
                    q.push({dis[v], v});
                    // }
                }
                
            }
        }
        vector<int> res;
        int tmp = end;
        // cout << start ;
        // 
        while (tmp != start) {
            res.push_back(tmp);
            tmp = parent[tmp];
        }
        res.push_back(start);
        reverse(res.begin(), res.end());
        return res;
    };
	void createNet();
	void createGraph();
	void createRing(int& nodeNum, list<pair<int, int>>& bridgeNodes, list<pair<int, int>>& sharedNodes, int bridgeNum = -1);
	void linkNodes(int& nodeNum, list<pair<int, int>>& bridgeNodes, list<pair<int, int>>& sharedNodes);
	void addEdges();
	void addEdge(int a, int b);
	bool isConnected();
	void visit(int k, vector<bool>& isVisited);
	void preprocessing();
	void recoveringNodes(int start, int end);
	void addRecoveringNode(int node);
	void addConnectedEdge(int a, int b, int dis, int ring);
	vector<pair<int, int>> getOrignalPath(int start, int end);
	vector<int> recoverPath(vector<pair<int, int>> orignalPath);
	vector<int> getPath(int start, int end);
	void print();
    vector<vector<int>> getGraph() {
        return this->graph;
    }

    
private:
	vector<vector<int>> graph;
	vector<vector<int>> rings;
	vector<pair<int, int>> bridges;
	vector<int> connectedNodes;
	vector<vector<int>> connectedEdges;
	int ringNum;
	int ringNodeNum;
	int connectedNodeNum;
};





int main(int argc, char* argv[]){
    Time::SetResolution(Time::NS);
    
    bool verbose = true;
    // uint32_t numNodes = 6;
    int ringNum = 5;
    int ringNodeNum = 15;
    int connectedNodeNum = 2;

    CommandLine cmd;   

    cmd.Usage("This project aims to simulate the shortest path routing algorithm in a multi-ring system.");
    cmd.AddValue("r", "Ring number in the generated graph", ringNum);
    cmd.AddValue("rn", "Node number in the generated graph", ringNodeNum);
    cmd.AddValue("cn", "Communication node number in the generated graph", connectedNodeNum);

    cmd.Parse(argc,argv);

    MultiRingNet test(ringNum, ringNodeNum, connectedNodeNum);
    test.createGraph();

    

    test.preprocessing();

    //命令行对象
    auto adjTable = test.getGraph();

    auto t = clock();
	
    vector<int> path = test.getPath(0, test.getGraph().size() - 1);
	
	cout << endl;
	NS_LOG_UNCOND("Multi ring algo time:");
    NS_LOG_UNCOND ( clock() - t);
	t = clock();
	auto djkstPath = test.dijkstra(0, test.getGraph().size() - 1);
	
	cout << endl;
	NS_LOG_UNCOND("Dijkstra algo time:");
	NS_LOG_UNCOND(clock() - t);
    // auto path = test.dijkstra(0, test.getGraph().size() - 1);
   
    cout << endl;
    auto numNodes = adjTable.size();
    // cout << numNodes << endl;
    if(verbose){
        LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);

        LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

        LogComponentEnable ("TestExample", LOG_LEVEL_INFO);

        //LogComponentEnable ("TcpL4Protocol", LOG_LEVEL_INFO);

        LogComponentEnable ("PacketSink", LOG_LEVEL_ALL);

        //LogComponentEnable ("OnOff", LOG_LEVEL_ALL)
    }







    //生成六个节点



    NodeContainer nodes;



    nodes.Create(numNodes);
	


    // 生成一个NodeContainer;
    vector<vector<int> > neighbour = adjTable;

    

    vector<NodeContainer> nodeContainers;

    // Todo: add a map
   //建立拓扑的各边节点组合，n1n2n3n4构成环

    map<vector<int>, int> edge2NodeIdx;
	// for (int i = 0; i < numNodes; i++) {
	// 	nodes.Get(i).SetConstantPosition();
	// }
    for (int i = 0; i < int(neighbour.size()); i++) {

        for (int j = 0; j < int(neighbour[i].size()); j++) {

            nodeContainers.push_back(NodeContainer(nodes.Get(i), nodes.Get(neighbour[i][j])));

            edge2NodeIdx[{i, neighbour[i][j]}] = nodeContainers.size() - 1;
            // cout << i << ' ' << neighbour[i][j] << '\n';
        }

    }
    


    //为所有节点安装协议栈



    InternetStackHelper internet;

    internet.SetIpv6StackInstall(false);
    internet.Install(nodes);

    //配置点到点连接
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate",StringValue("1Mbps"));//网卡最大速率
    p2p.SetChannelAttribute("Delay",StringValue("2ms"));

    //为链路安装点到点连接

    NetDeviceContainer nets;


    vector<NetDeviceContainer> edges;

    for (auto nodeContainer : nodeContainers) {

        edges.push_back(p2p.Install(nodeContainer));

    }



    //为链路设置ip地址



    Ipv4AddressHelper ipv4;

    auto subNetMask = "255.255.255.0";

    vector<Ipv4InterfaceContainer> interfaceContainers;
    for (int i = 0; i < int(edges.size()); i++) {
		string ipAddress;
		
		ipAddress = "10." + to_string((i % (256 * 256)) / 256) + "." + to_string((i % (256 * 256)) % 256) + ".0"  ;
		// cout << ipAddress << endl;
        ipv4.SetBase(ipAddress.c_str(), subNetMask);

        auto interfaceContainer = ipv4.Assign(edges[i]);

        interfaceContainers.push_back(interfaceContainer);

    }

    
    //配置应用层

    uint16_t port = 9;

    UdpEchoServerHelper echoServer (port);

    ApplicationContainer serverApps = echoServer.Install(nodes.Get(path[1]));
    for (int i = 2; i < int(path.size()); i++) {
        serverApps.Add(echoServer.Install( nodes.Get(path[i])));
    }

    serverApps.Start(Seconds(1.0));

    serverApps.Stop(Seconds(path.size()));

    vector<UdpEchoClientHelper> echoClients;

    for (int i = 0; i < int(path.size()) - 1; i++) {

        auto nodeIdx = edge2NodeIdx[{path[i], path[i + 1]}];
        auto echoClient = UdpEchoClientHelper( interfaceContainers[nodeIdx].GetAddress(1), port );
        echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
        echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
        echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

        ApplicationContainer clientApp = echoClient.Install (nodes.Get (path[i]));
        clientApp.Start(Seconds(i + 2));
        clientApp.Stop(Seconds(i + 3));
        echoClients.push_back(echoClient);
    }


    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


    AnimationInterface anim ("xml/test.xml");
	// anim.SetConstantPosition
    Simulator::Run();

    Simulator::Destroy();

    return 0;           



}   

void MultiRingNet::createGraph()
{
	while (!isConnected())
	{
		srand(static_cast<unsigned int>(time(0)));
		int nodeNum = 0;
		rings.clear();
		bridges.clear();
		list<pair<int, int>> bridgeNodes, sharedNodes;
		for (int i = 1; i < ringNum; ++i)
		{
			createRing(nodeNum, bridgeNodes, sharedNodes);
		}
		linkNodes(nodeNum, bridgeNodes, sharedNodes);
		graph = vector<vector<int>>(nodeNum);
		addEdges();
	}
	preprocessing();
}

void MultiRingNet::createRing(int& nodeNum, list<pair<int, int>>& bridgeNodes, list<pair<int, int>>& sharedNodes, int bridgeNum)
{
	int curRing = rings.size();
	rings.push_back(vector<int>(ringNodeNum, 0));
	for (int i = 0; i < connectedNodeNum; ++i)
	{
		int pos = rand() % ringNodeNum;
		while (rings.back()[pos])
			pos = rand() % ringNodeNum;
		rings.back()[pos] = -1;
	}
	int isBridge = RAND_MAX / 2;
	for (int i = 0; i < ringNodeNum; ++i)
	{
		if (rings.back()[i])
		{
			if ((bridgeNum > 0 && bridgeNum--) || (bridgeNum < 0 && rand() < isBridge))
			{
				bridgeNodes.push_back(pair<int, int>(curRing, i));
			}
			else
			{
				sharedNodes.push_back(pair<int, int>(curRing, i));
				continue;
			}
		}
		rings.back()[i] = nodeNum++;
	}
}

void MultiRingNet::linkNodes(int& nodeNum, list<pair<int, int>>& bridgeNodes, list<pair<int, int>>& sharedNodes)
{
	while (bridgeNodes.size())
	{
		if (bridgeNodes.front().first == bridgeNodes.back().first)
		{
			createRing(nodeNum, bridgeNodes, sharedNodes, bridgeNodes.size());
			linkNodes(nodeNum, bridgeNodes, sharedNodes);
			break;
		}
		list<pair<int, int>>::iterator anotherNode = bridgeNodes.begin();
		advance(anotherNode, rand() % bridgeNodes.size());
		while (bridgeNodes.front().first == anotherNode->first)
		{
			anotherNode = bridgeNodes.begin();
			advance(anotherNode, rand() % bridgeNodes.size());
		}
		bridges.push_back(pair<int, int>(rings[bridgeNodes.front().first][bridgeNodes.front().second], rings[anotherNode->first][anotherNode->second]));
		bridgeNodes.erase(anotherNode);
		bridgeNodes.pop_front();
	}
	if (int(rings.size()) < ringNum)
	{
		createRing(nodeNum, bridgeNodes, sharedNodes, 0);
	}
	while (sharedNodes.size())
	{
		if (sharedNodes.front().first == sharedNodes.back().first)
		{
			for (auto node : sharedNodes)
			{
				rings[node.first][node.second] = nodeNum++;
			}
			sharedNodes.clear();
			break;
		}
		list<pair<int, int>>::iterator anotherNode = sharedNodes.begin();
		advance(anotherNode, rand() % sharedNodes.size());
		while (sharedNodes.front().first == anotherNode->first)
		{
			anotherNode = sharedNodes.begin();
			advance(anotherNode, rand() % sharedNodes.size());
		}
		rings[sharedNodes.front().first][sharedNodes.front().second] = rings[anotherNode->first][anotherNode->second] = nodeNum++;
		sharedNodes.erase(anotherNode);
		sharedNodes.pop_front();
	}
}

void MultiRingNet::addEdges()
{
	for (int i = 0; i < ringNum; ++i)
	{
		for (int j = 1; j < ringNodeNum; ++j)
		{
			addEdge(rings[i][j], rings[i][j - 1]);
		}
		addEdge(rings[i].front(), rings[i].back());
	}
	for (auto bridge : bridges)
	{
		addEdge(bridge.first, bridge.second);
	}
}

void MultiRingNet::addEdge(int a, int b)
{
	if (find(graph[a].begin(), graph[a].end(), b) == graph[a].end())
	{
		graph[a].push_back(b);
	}
	if (find(graph[b].begin(), graph[b].end(), a) == graph[b].end())
	{
		graph[b].push_back(a);
	}
}

bool MultiRingNet::isConnected()
{
	if (int(graph.size()) == 0)
	{
		return false;
	}
	vector<bool> isVisited = vector<bool>(graph.size());
	visit(0, isVisited);
	for (auto i : isVisited)
	{
		if (!i)
		{
			return false;
		}
	}
	return true;
}

void MultiRingNet::visit(int k, vector<bool>& isVisited)
{
	isVisited[k] = true;
	for (auto node : graph[k])
	{
		if (!isVisited[node])
		{
			visit(node, isVisited);
		}
	}
}

void MultiRingNet::preprocessing()
{
	for (int i = 0; i < ringNum; ++i)
	{
		vector<pair<int, int>> ringConnectedNodes;
		for (int j = 0; j < ringNodeNum; ++j)
			if (int(graph[rings[i][j]].size()) > 2)
			{
				if (find(connectedNodes.begin(), connectedNodes.end(), rings[i][j]) == connectedNodes.end())
				{
					connectedNodes.push_back(rings[i][j]);
				}
				for (auto node : ringConnectedNodes)
				{
					addConnectedEdge(rings[i][j], node.second, j - node.first, i);
				}
				ringConnectedNodes.push_back(pair<int, int>(j, rings[i][j]));
			}
	}
	for (auto bridge : bridges)
	{
		addConnectedEdge(bridge.first, bridge.second, 1, -1);
	}
}

void MultiRingNet::addConnectedEdge(int a, int b, int dis, int ring)
{
	if (a > b)
	{
		swap(a, b);
	}
	dis = min(dis, ringNodeNum - dis);
	for (auto edge : connectedEdges)
	{
		if (edge[0] == a && edge[1] == b)
		{
			if (dis < edge[2])
			{
				edge[2] = dis;
				edge[3] = ring;
			}
			return;
		}
	}
	vector<int> edge = { a, b, dis, ring };
	connectedEdges.push_back(edge);
}

void MultiRingNet::recoveringNodes(int start, int end)
{
	if (find(connectedNodes.begin(), connectedNodes.end(), start) == connectedNodes.end())
	{
		addRecoveringNode(start);
	}
	if (find(connectedNodes.begin(), connectedNodes.end(), end) == connectedNodes.end())
	{
		addRecoveringNode(end);
	}
}

void MultiRingNet::addRecoveringNode(int node)
{
	for (int i = 0; i < int(rings.size()); ++i)
	{
		int pos = find(rings[i].begin(), rings[i].end(), node) - rings[i].begin();
		if (pos != int(rings[i].size()))
		{
			for (int j = 0; j < int(rings[i].size()); ++j)
			{
				if (find(connectedNodes.begin(), connectedNodes.end(), rings[i][j]) != connectedNodes.end())
				{
					addConnectedEdge(node, rings[i][j], abs(j - pos), i);
				}
			}
			break;
		}
	}
	connectedNodes.push_back(node);
}

vector<pair<int, int>> MultiRingNet::getOrignalPath(int start, int end)
{
	map<int, vector<pair<int, int>>> connectedGraph;
	for (auto node : connectedNodes)
	{
		connectedGraph[node] = vector<pair<int, int>>();
	}
	for (int i = 0; i < int(connectedEdges.size()); ++i)
	{
		if (connectedGraph.count(connectedEdges[i][0]) && connectedGraph.count(connectedEdges[i][1]))
		{
			connectedGraph[connectedEdges[i][0]].push_back(pair<int, int>(connectedEdges[i][1], i));
			connectedGraph[connectedEdges[i][1]].push_back(pair<int, int>(connectedEdges[i][0], i));
		}
	}
	map<int, pair<int, int>> disMap;
	map<int, int> nearest;
	for (auto node : connectedNodes)
	{
		disMap[node] = pair<int, int>(INT_MAX, -1);
		nearest[node] = -1;
	}
	disMap[start].first = -1;
	for (auto& edge : connectedGraph[start])
	{
		disMap[edge.first].first = connectedEdges[edge.second][2];
		disMap[edge.first].second = connectedEdges[edge.second][3];
		nearest[edge.first] = start;
	}
	int curNode = start;
	while (curNode != end)
	{
		int selectNode = -1, minDis = INT_MAX;
		for (int i = 0; i < int(connectedNodes.size()); ++i)
		{
			if (disMap[connectedNodes[i]].first != -1 && disMap[connectedNodes[i]].first < minDis)
			{
				minDis = disMap[connectedNodes[i]].first;
				selectNode = connectedNodes[i];
			}
		}
		for (auto& edge : connectedGraph[selectNode])
		{
			if (disMap[edge.first].first > connectedEdges[edge.second][2] + disMap[selectNode].first)
			{
				disMap[edge.first].first = connectedEdges[edge.second][2] + disMap[selectNode].first;
				disMap[edge.first].second = connectedEdges[edge.second][3];
				nearest[edge.first] = selectNode;
			}
		}
		disMap[selectNode].first = -1;
		curNode = selectNode;
	}
	vector<pair<int, int>> orignalPath;
	while (curNode != start)
	{
		orignalPath.push_back(pair<int, int>(curNode, disMap[curNode].second));
		curNode = nearest[curNode];
	}
	orignalPath.push_back(pair<int, int>(curNode, -1));
	reverse(orignalPath.begin(), orignalPath.end());
	return orignalPath;
}

vector<int> MultiRingNet::recoverPath(vector<pair<int, int>> orignalPath)
{
	vector<int> path;
	for (int i = 1; i < int(orignalPath.size()); ++i)
	{
		int r = orignalPath[i].second;
		if (r == -1)
		{
			path.push_back(orignalPath[i - 1].first);
			continue;
		}
		int start = find(rings[r].begin(), rings[r].end(), orignalPath[i - 1].first) - rings[r].begin();
		int end = find(rings[r].begin(), rings[r].end(), orignalPath[i].first) - rings[r].begin();
		if (start < end)
		{
			int len = end - start;
			if (len < ringNodeNum - len)
			{
				for (int i = start; i < end; ++i)
				{
					path.push_back(rings[r][i]);
				}
			}
			else
			{
				for (int i = start; i >= 0; --i)
				{
					path.push_back(rings[r][i]);
				}
				for (int i = ringNodeNum - 1; i > end; --i)
				{
					path.push_back(rings[r][i]);
				}
			}
		}
		else
		{
			int len = start - end;
			if (len < ringNodeNum - len)
			{
				for (int i = start; i > end; --i)
				{
					path.push_back(rings[r][i]);
				}
			}
			else
			{
				for (int i = start; i < ringNodeNum; ++i)
				{
					path.push_back(rings[r][i]);
				}
				for (int i = 0; i < end; ++i)
				{
					path.push_back(rings[r][i]);
				}
			}
		}
	}
	path.push_back(orignalPath.back().first);
	return path;
}

vector<int> MultiRingNet::getPath(int start, int end)
{
	int nodeNum = connectedNodes.size(), edgeNum = connectedEdges.size();
	recoveringNodes(start, end);
	vector<int> path = recoverPath(getOrignalPath(start, end));
	while (int(connectedNodes.size()) > nodeNum)
	{
		connectedNodes.pop_back();
	}
	while (int(connectedEdges.size()) > edgeNum)
	{
		connectedEdges.pop_back();
	}
	return path;
}

void MultiRingNet::print()
{
	clock_t cost = clock();
	vector<int> ans = getPath(0, graph.size() - 1);
	cost = clock() - cost;
	cout << cost << endl << endl;
	cout << ans.size() << endl << endl;
	for (int i = 0; i < int(ans.size()); ++i)
	{
		cout << ans[i] << endl;
	}
}
