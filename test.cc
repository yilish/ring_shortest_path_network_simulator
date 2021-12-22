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
	vector<int> dijkstra();
	void createNet();
	void createGraph();
	void createRing(int& nodeNum, list<pair<int, int> >& bridgeNodes, list<pair<int, int> >& sharedNodes, int bridgeNum = -1);
	void linkNodes(int& nodeNum, list<pair<int, int> >& bridgeNodes, list<pair<int, int> >& sharedNodes);
	void addEdges();
	void addEdge(int a, int b);
	bool isConnected();
	void visit(int k, vector<bool>& isVisited);
	void preprocessing();
	void recoveringNodes(int start, int end);
	void addConnectedEdge(int a, int b, int dis, int ring);
	void print();
    
    
    void showGraphContent();
    vector<vector<int> > getGraphNeighbourTable();

private:
	vector<vector<int> > graph;
	vector<vector<int> > rings;
	vector<pair<int, int> > bridges;
	vector<int> connectedNodes;
	vector<vector<int> > connectedEdges;
	int ringNum;
	int ringNodeNum;
	int connectedNodeNum;
};


int main(int argc, char* argv[]){

    Time::SetResolution(Time::NS);

    bool verbose = true;

    uint32_t numNodes = 6;
    MultiRingNet test(5, 5, 2);
    test.createGraph();
    
    test.preprocessing();
	// test.print();   
    // test.showGraphContent();
    //命令行对象

    CommandLine cmd;   

    cmd.Parse(argc,argv);


    if(verbose){

        LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);

        LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

        LogComponentEnable ("TestExample", LOG_LEVEL_INFO);

        //LogComponentEnable ("TcpL4Protocol", LOG_LEVEL_INFO);

        LogComponentEnable ("PacketSink", LOG_LEVEL_ALL);

        //LogComponentEnable ("OnOff", LOG_LEVEL_ALL);

    }



    //生成六个节点

    NodeContainer nodes;

    nodes.Create(numNodes);

    // 生成一个NodeContainer;
    vector<vector<int> > neighbour;
    neighbour.push_back({1});
    neighbour.push_back({2, 3});
    neighbour.push_back({4});
    neighbour.push_back({4});
    neighbour.push_back({5});
    //  = {{1}, {2, 3}, {4}, {4}, {5}};
    
    vector<NodeContainer> nodeContainers;
    // Todo: add a map
    map<vector<int>, int> edge2NodeIdx;
    for (int i = 0; i < neighbour.size(); i++) {
        for (int j = 0; j < neighbour[i].size(); j++) {
            nodeContainers.push_back(NodeContainer(nodes.Get(i), nodes.Get(neighbour[i][j])));
            edge2NodeIdx[{i, neighbour[i][j]}] = nodeContainers.size() - 1;
            cout << i << ' ' << neighbour[i][j] << '\n';
        }
    }

    

    //建立拓扑的各边节点组合，n1n2n3n4构成环

    // NodeContainer n0n1 = NodeContainer(nodes.Get(0),nodes.Get(1));

    // NodeContainer n1n2 = NodeContainer(nodes.Get(1),nodes.Get(2));

    // NodeContainer n1n3 = NodeContainer(nodes.Get(1),nodes.Get(3));

    // NodeContainer n2n4 = NodeContainer(nodes.Get(2),nodes.Get(4));

    // NodeContainer n3n4 = NodeContainer(nodes.Get(3),nodes.Get(4));

    // NodeContainer n4n5 = NodeContainer(nodes.Get(4),nodes.Get(5));



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
    // NetDeviceContainer d0d1 = p2p.Install(n0n1);

    // NetDeviceContainer d1d2 = p2p.Install(n1n2);

    // NetDeviceContainer d1d3 = p2p.Install(n1n3);

    // NetDeviceContainer d2d4 = p2p.Install(n2n4);

    // NetDeviceContainer d3d4 = p2p.Install(n3n4);

    // NetDeviceContainer d4d5 = p2p.Install(n4n5);    



    //为链路设置ip地址

    Ipv4AddressHelper ipv4;
    auto subNetMask = "255.255.255.0";
    vector<Ipv4InterfaceContainer> interfaceContainers;
    for (int i = 0; i < edges.size(); i++) {
        string ipAddress = "10.1." + to_string(i) + ".0";
        ipv4.SetBase(ipAddress.c_str(), subNetMask);
        auto interfaceContainer = ipv4.Assign(edges[i]);
        interfaceContainers.push_back(interfaceContainer);
    }
    // string add = "10.1.1.0";
    // ipv4.SetBase( "10.1.1.0", "255.255.255.0");

    // Ipv4InterfaceContainer i0i1 = ipv4.Assign(d0d1);

    // ipv4.SetBase("10.1.2.0","255.255.255.0");

    // Ipv4InterfaceContainer i1i2 = ipv4.Assign(d1d2);

    // ipv4.SetBase("10.1.3.0","255.255.255.0");

    // Ipv4InterfaceContainer i1i3 = ipv4.Assign(d1d3);

    // ipv4.SetBase("10.1.4.0","255.255.255.0");

    // Ipv4InterfaceContainer i2i4 = ipv4.Assign(d2d4);

    // ipv4.SetBase("10.1.5.0","255.255.255.0");

    // Ipv4InterfaceContainer i3i4 = ipv4.Assign(d3d4);

    // ipv4.SetBase("10.1.6.0","255.255.255.0");

    // Ipv4InterfaceContainer i4i5 = ipv4.Assign(d4d5);

 

    
    vector<int> path = {0, 1, 2, 4, 5};
    //配置应用层
    uint16_t port = 9;
    UdpEchoServerHelper echoServer (port);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(path[1]));
    for (int i = 2; i < path.size(); i++) {
        serverApps.Add(echoServer.Install( nodes.Get(path[i])) );
    }
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(path.size()));
    // ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
    // serverApps.Add(echoServer.Install(nodes.Get(2)));
    // serverApps.Add(echoServer.Install(nodes.Get(4)));
    // serverApps.Add(echoServer.Install(nodes.Get(5)));
    // serverApps.Start (Seconds (1.0));
    // serverApps.Stop (Seconds (10.0));
    vector<UdpEchoClientHelper> echoClients;
    for (int i = 0; i < path.size() - 1; i++) {
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


    // //n0->n1
    // UdpEchoClientHelper echoClient1 (i0i1.GetAddress(1), port);
    // echoClient1.SetAttribute ("MaxPackets", UintegerValue (1));
    // echoClient1.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    // echoClient1.SetAttribute ("PacketSize", UintegerValue (1024));

    // ApplicationContainer clientApps1 = echoClient1.Install (nodes.Get (0));
    // clientApps1.Start (Seconds (2.0));
    // clientApps1.Stop (Seconds (3.0));

    // //n1->n2
    // UdpEchoClientHelper echoClient2 (i1i2.GetAddress(1), port);
    // echoClient2.SetAttribute ("MaxPackets", UintegerValue (1));
    // echoClient2.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    // echoClient2.SetAttribute ("PacketSize", UintegerValue (1024));

    // ApplicationContainer clientApps2 = echoClient2.Install (nodes.Get (1));
    // clientApps2.Start (Seconds (3.0));
    // clientApps2.Stop (Seconds (4.0));

    // //n2->n4
    // UdpEchoClientHelper echoClient3 (i2i4.GetAddress(1), port);
    // echoClient3.SetAttribute ("MaxPackets", UintegerValue (1));
    // echoClient3.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    // echoClient3.SetAttribute ("PacketSize", UintegerValue (1024));

    // ApplicationContainer clientApps3 = echoClient3.Install (nodes.Get (2));
    // clientApps3.Start (Seconds (4.0));
    // clientApps3.Stop (Seconds (5.0));

    // //n4->n5
    // UdpEchoClientHelper echoClient4 (i4i5.GetAddress(1), port);
    // echoClient4.SetAttribute ("MaxPackets", UintegerValue (1));
    // echoClient4.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    // echoClient4.SetAttribute ("PacketSize", UintegerValue (1024));

    // ApplicationContainer clientApps4 = echoClient4.Install (nodes.Get (4)); // 起点节点
    // clientApps4.Start (Seconds (5.0));
    // clientApps4.Stop (Seconds (6.0));
    /* uint16_t port = 50000;

    ApplicationContainer sinkApp;

    Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));

    PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);

    sinkApp.Add(sinkHelper.Install(nodes.Get(0)));
    sinkApp.Add(sinkHelper.Install(nodes.Get(1)));
    sinkApp.Add(sinkHelper.Install(nodes.Get(2)));
    sinkApp.Add(sinkHelper.Install(nodes.Get(4)));

    sinkApp.Start (Seconds (0.0));

    sinkApp.Stop (Seconds (3.0)); 



    OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address ());

    clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));

    clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    //clientHelper.SetAttribute ("PacketSize", UintegerValue (1024));
    //clientHelper.SetAttribute ("DataRate", StringValue ("10kb/s"));


    ApplicationContainer clientApps1, clientApps2, clientApps3, clientApps4;

    //n0->n1

    AddressValue remoteAddress(InetSocketAddress (i0i1.GetAddress (1), port));

    clientHelper.SetAttribute("Remote",remoteAddress);

    clientApps1.Add(clientHelper.Install(nodes.Get(0)));

    //n1->n2

    remoteAddress = AddressValue(InetSocketAddress (i1i2.GetAddress (1), port));

    clientHelper.SetAttribute("Remote",remoteAddress);

    clientApps2.Add(clientHelper.Install(nodes.Get(1)));

    //n2->n4

    remoteAddress = AddressValue(InetSocketAddress (i2i4.GetAddress (1), port));

    clientHelper.SetAttribute("Remote",remoteAddress);

    clientApps3.Add(clientHelper.Install(nodes.Get(2)));

    //n4->n5

    remoteAddress = AddressValue(InetSocketAddress (i4i5.GetAddress (1), port));

    clientHelper.SetAttribute("Remote",remoteAddress);

    clientApps4.Add(clientHelper.Install(nodes.Get(4)));



    clientApps1.Start(Seconds(1.0));

    clientApps1.Stop (Seconds (1.1));

    clientApps2.Start(Seconds(1.2));

    clientApps2.Stop (Seconds (1.3));

    clientApps3.Start(Seconds(1.4));

    clientApps3.Stop (Seconds (1.5));

    clientApps4.Start(Seconds(1.6));

    clientApps4.Stop (Seconds (1.7)); */



    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();



    AnimationInterface anim ("xml/test.xml");

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
		list<pair<int, int> > bridgeNodes, sharedNodes;
		for (int i = 1; i < ringNum; ++i)
			createRing(nodeNum, bridgeNodes, sharedNodes);
		linkNodes(nodeNum, bridgeNodes, sharedNodes);
		graph = vector<vector<int> >(nodeNum);
		addEdges();
	}
}

void MultiRingNet::createRing(int& nodeNum, list<pair<int, int> >& bridgeNodes, list<pair<int, int> >& sharedNodes, int bridgeNum)
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
		if (rings.back()[i]) {
			if ((bridgeNum > 0 && bridgeNum--) || (bridgeNum < 0 && rand() < isBridge)){
				bridgeNodes.push_back(pair<int, int>(curRing, i));
            }
            else {
				sharedNodes.push_back(pair<int, int>(curRing, i));
				continue;
			}
        }
		rings.back()[i] = nodeNum++;
	}
}

void MultiRingNet::linkNodes(int& nodeNum, list<pair<int, int> >& bridgeNodes, list<pair<int, int> >& sharedNodes)
{
	while (bridgeNodes.size())
	{
		if (bridgeNodes.front().first == bridgeNodes.back().first)
		{
			createRing(nodeNum, bridgeNodes, sharedNodes, bridgeNodes.size());
			linkNodes(nodeNum, bridgeNodes, sharedNodes);
			break;
		}
		list<pair<int, int> >::iterator anotherNode = bridgeNodes.begin();
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
	if (rings.size() < ringNum)
		createRing(nodeNum, bridgeNodes, sharedNodes, 0);
	while (sharedNodes.size())
	{
		if (sharedNodes.front().first == sharedNodes.back().first)
		{
			for (auto node : sharedNodes)
				rings[node.first][node.second] = nodeNum++;
			sharedNodes.clear();
			break;
		}
		list<pair<int, int> >::iterator anotherNode = sharedNodes.begin();
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
			addEdge(rings[i][j], rings[i][j - 1]);
		addEdge(rings[i].front(), rings[i].back());
	}
	for (auto bridge : bridges)
		addEdge(bridge.first, bridge.second);
}

void MultiRingNet::addEdge(int a, int b)
{
	if (find(graph[a].begin(), graph[a].end(), b) == graph[a].end())
		graph[a].push_back(b);
	if (find(graph[b].begin(), graph[b].end(), a) == graph[b].end())
		graph[b].push_back(a);
}

bool MultiRingNet::isConnected()
{
	if (graph.size() == 0)
		return false;
	vector<bool> isVisited = vector<bool>(graph.size());
	visit(0, isVisited);
	for (auto i : isVisited)
		if (!i)
			return false;
	return true;
}

void MultiRingNet::visit(int k, vector<bool>& isVisited)
{
	isVisited[k] = true;
	for (auto node : graph[k])
		if (!isVisited[node])
			visit(node, isVisited);
}

void MultiRingNet::preprocessing()
{
	for (int i = 0; i < ringNum; ++i)
	{
		vector<pair<int, int> > ringConnectedNodes = vector<pair<int, int> >();
		for (int j = 0; j < ringNodeNum; ++j)
			if (graph[rings[i][j]].size() > 2 && find(connectedNodes.begin(), connectedNodes.end(), rings[i][j]) == connectedNodes.end())
			{
				connectedNodes.push_back(rings[i][j]);
				for (auto node : ringConnectedNodes)
					addConnectedEdge(rings[i][j], node.second, j - node.first, i);
				ringConnectedNodes.push_back(pair<int, int>(j, rings[i][j]));
			}
	}
	for (auto bridge : bridges)
		addConnectedEdge(bridge.first, bridge.second, 1, -1);
}

void MultiRingNet::recoveringNodes(int start, int end)
{
	if (find(connectedNodes.begin(), connectedNodes.end(), start) == connectedNodes.end())
	{
		connectedNodes.push_back(start);
		for(auto ring : rings)
			if (find(ring.begin(), ring.end(), start) != ring.end())
			{
				break;
			}
	}
}

void MultiRingNet::addConnectedEdge(int a, int b, int dis, int ring)
{
	if (a > b)
		swap(a, b);
	dis = min(dis, ringNodeNum - dis);
	for(auto edge : connectedEdges)
		if (edge[0] == a && edge[1] == b)
		{
			if (dis < edge[2])
			{
				edge[2] = dis;
				edge[3] = ring;
			}
			return;
		}
	vector<int> edge;
	edge.push_back(a);
	edge.push_back(b);
	edge.push_back(dis);
	edge.push_back(ring);
	// for (auto item: { a, b, dis, ring }) {
	// 	edge.push_back(item);
	// }

	connectedEdges.push_back(edge);
}

void MultiRingNet::print()
{
	cout << graph.size() << endl;
	for (int i = 0; i < ringNum; ++i)
		cout << rings[i].size() << endl;
	cout << connectedNodes.size() << endl;
	cout << connectedEdges.size() << endl;
}

void MultiRingNet::showGraphContent() {
    for (auto vec : this->graph) {
        cout << vec.size() << ' ';
    }
}

vector<vector<int> > MultiRingNet::getGraphNeighbourTable() {
    return this->graph;
} 