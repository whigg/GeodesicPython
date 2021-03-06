#include "Node.h"
#include "Grid.h"
#include "math_functions.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <stdio.h>

struct AngleSort{
  double ang;
  Node * node;

  bool operator<( const AngleSort& rhs ) const { return ang < rhs.ang; }
};

Grid::Grid(void)
{
  this->recursion_lvl = 0;
};

void Grid::addNode(Node *n)
{
    node_list.push_back(n);
};

void Grid::findFriends(void)
{
    int i, j;
    double dist;
    Node * nodeA, * nodeB;
    Node * node, * node_friend;

    for (i=0; i<node_list.size(); i++)
    {

        std::vector<int> surrounding_nodes;

        nodeA = this->node_list[i];
        for (j=0; j<node_list.size(); j++)
        {
            if (i != j)
            {
                nodeB = this->node_list[j];

                dist = (*nodeA - *nodeB).getMagnitude();

                // std::cout<<dist<<std::endl;

                if (dist < 1.06)
                {
                    surrounding_nodes.push_back( nodeB->ID );
                    if (nodeA->friends_list.size()<5) nodeA->addFriend( nodeB );
                    // if (nodeB->friends_list.size()<5) nodeB->addFriend( nodeA );
                }
            }
        }

        surrounding_nodes.push_back(-1);
        nodeA->friends_list.push_back(NULL);

        this->friends_list.push_back( surrounding_nodes );
    }

    double dot_prod;
    double det;
    double mag1, mag2;
    double cosx, rad;
    double inner_angle;

    // std::cout<<'\t'<<"Ordering friends of new nodes..."<<std::flush;
    for (i=0; i<node_list.size(); i++)
    {
      //Order the friends of the new guys
      // double v1[2], v2[2];

      std::vector<AngleSort> ordered_friends(5);
      double v1[2] = {0., 1.};
      double v2[2];
      double angle;

      node = node_list[i];
      for (j=0; j<5; j++)
      {
        // std::cout<<node->friends_list.size()<<std::endl;
        node_friend = node->friends_list[j];
        node_friend->getMapCoords(*node, v2);

        dot_prod = v1[0]*v2[0] + v1[1]*v2[1];

        det = v1[0]*v2[1] - v1[1]*v2[0];

        ordered_friends[j].ang = atan2(dot_prod, det)*180./pi;

        ordered_friends[j].node = node_friend;
      }

      std::sort(ordered_friends.begin(), ordered_friends.end());

      for (j=0; j<5; j++)
      {
        node->friends_list[j] = ordered_friends[j].node;
      }

    }
};

void Grid::bisectEdges(void)
{
    int node_count, i, j, k, added;
    double min_dist = 1.0;
    double dist;
    Node * node, *node_friend, *node_friend2;
    Node * inter_friend1, * inter_friend2;
    std::vector< Node * > new_nodes;

    this->recursion_lvl += 1;

    node_count = this->node_list.size()-1;

    std::cout<<'\t'<<"Beginning bisection..."<<std::flush;
    for (i=0; i<node_list.size(); i++)
    {
        node = this->node_list[i];
        for (j=0; j<node->friend_num; j++)
        {
            Node * new_node;
            if (node->updated[j] == 0)
            {
                node_friend = node->friends_list[j];

                new_node = *(*node + *node_friend) * 0.5;

                node_count += 1;
                new_node->project2Sphere();
                new_node->ID = node_count;
                new_node->friend_num = 6;

                min_dist = std::min(min_dist, (*node - *new_node).getMagnitude());

                node->addTempFriend(new_node);
                new_nodes.push_back(new_node);

                new_node->addFriend(node);
                new_node->addFriend(node_friend);

                for (k=0; k<node_friend->friend_num; k++)
                {
                    node_friend2 = node_friend->friends_list[k];

                    if (*node == *node_friend2)
                    {
                        node_friend->addTempFriend(new_node);
                        node_friend->updated[k] = 1;
                        break;
                    }
                }

                node->updated[j] = 1;

            }
        }
    }
    std::cout<<"\r\tBeginning bisection... complete!"<<std::endl;

    std::cout<<'\t'<<"Determining all friends of new nodes..."<<std::flush;
    for (i=0; i<node_list.size(); i++)
    {
        node = this->node_list[i];

        for (j=0; j<node->friend_num; j++)
        {
            inter_friend1 = node->temp_friends[j];

            for (k=0; k<node->friend_num; k++)
            {
              inter_friend2 = node->temp_friends[k];

              if (*inter_friend1 != *inter_friend2)
              {
                dist = (*inter_friend1 - *inter_friend2).getMagnitude();

                if (dist < min_dist*1.2)
                {
                  inter_friend1->addFriend(inter_friend2);
                }
              }
            }
        }
    }
    std::cout<<'\r'<<"\tDetermining all friends of new nodes... complete!"<<std::endl;

    std::cout<<'\t'<<"Assigning new nodes to old nodes..."<<std::flush;
    for (i=0; i<node_list.size(); i++)
    {
        node = this->node_list[i];
        for (j=0; j<node->friend_num; j++)
        {
            node->friends_list[j] = node->temp_friends[j];
            node->updated[j] = 0;
        }

        node->temp_friends.clear();
    }
    std::cout<<'\r'<<"\tAssigning new nodes to old nodes... complete!"<<std::endl;

    std::cout<<'\t'<<"Adding new nodes to master list..."<<std::flush;
    node_list.insert(node_list.end(),new_nodes.begin(),new_nodes.end());
    std::cout<<'\r'<<"\tAdding new nodes to master list... complete!"<<std::endl;

};

void Grid::orderFriends(void)
{
    int i, j, k;
    Node * node, *node_friend, *node_friend2;
    double v2[2];

    // std::cout<<'\t'<<"Ordering friends of new nodes..."<<std::flush;
    for (i=0; i<node_list.size(); i++)
    {
      //Order the friends of the new guys
      // double v1[2], v2[2];
      node = node_list[i];

      std::vector<AngleSort> ordered_friends(node->friend_num);


      for (j=0; j<node->friend_num; j++)
      {
        node_friend = node->friends_list[j];
        node_friend->getMapCoords(*node, v2);

        ordered_friends[j].ang = atan2(v2[0], v2[1])*180./pi;// - 90.0 + 360.0;

        ordered_friends[j].node = node_friend;
      }

      std::sort(ordered_friends.begin(), ordered_friends.end());

      for (j=0; j<node->friend_num; j++)
      {
        node->friends_list[j] = ordered_friends[j].node;
      }

    }
}

void Grid::shiftNodes(void)
{
    int i, j, k;
    double mag;
    Node * node1, *node2, *node3;
    double sph1[3], sph2[3], sph3[3];
    double xy_sub_centers[6][3];
    double xy1[3], xy2[3], xy3[3];
    double xy_new_center[3];
    double new_xyz[3];
    double diff_xyz[k];
    double sph_center[3];
    double areas[6];
    double area;
    std::vector<std::vector<double>> shifted_xyz(node_list.size(), std::vector<double> (3));
    double residual = 1.0;
    double residual_old = 0.0;
    double e_converge = 1e-20;
    double r = 0.0;
    int iter;

    // while loop --> until converged

    // call find centroid function
    // for loop over every node
        // compute control volume areas
        // find the shifted node point
        // compare shifted point to old point to find residual
        // update old node
        // call find centroid function
    iter = 0;
    this->findCentroids();
    while ((iter < 2000) && residual > e_converge)//(residual > e_converge)
    {
        residual_old = residual;
        residual = 0.0;
        for (i=0; i<node_list.size(); i++)
        {
            node1 = this->node_list[i];

            for (k=0; k<3; k++) {
                sph1[k] = node1->sph_coords[k];    //latitude
                xy1[k] = node1->xyz_coords[k];
            }

            xy_new_center[0] = 0.0;
            xy_new_center[1] = 0.0;
            xy_new_center[2] = 0.0;

            for (j=0; j<node1->friend_num; j++)
            {

                for (k=0; k<3; k++)
                {
                    sph2[k] = node1->centroids[j][k];
                    sph3[k] = node1->centroids[(j+1)%node1->friend_num][k];
                }

                areas[j] = sphericalArea(sph1, sph2, sph3);
                // std::cout<<j<<'\t'<<areas[j]<<'\t'<<sph1[1]*180./pi<<'\t'<<sph2[1]*180./pi<<'\t'<<sph3[1]*180./pi<<std::endl;
                // sph1[1] = pi*0.5 - sph1[1];
                sph2[1] = pi*0.5 - sph2[1];
                sph3[1] = pi*0.5 - sph3[1];

                sph2cart(sph2, xy2);
                sph2cart(sph3, xy3);

                for (k=0; k<3; k++)
                    xy_new_center[k] += areas[j]*(xy1[k] + xy2[k] + xy3[k]);



            }

            mag = 0.0;
            for (k=0; k<3; k++) mag += xy_new_center[k]*xy_new_center[k];
            mag = sqrt(mag);

            r = 0.0;
            for (k=0; k<3; k++) {
                xy_new_center[k] /= mag;

                r += (xy_new_center[k] - xy1[k])*(xy_new_center[k] - xy1[k]);
            }
            r = sqrt(r);

            //if (node1->friend_num == 6)
            //{
                residual += r*r;
                node1 = this->node_list[i];
                node1->updateXYZ(xy_new_center);
            //}

        }

        std::cout<<iter<<'\t'<<residual<<std::endl;

        this->findCentroids();

        iter += 1;


    }


    // for (i=0; i<node_list.size(); i++)
    // {
    //     node1 = this->node_list[i];
    //     for (k=0; k<3; k++) {
    //         sph1[k] = node1->sph_coords[k];    //latitude
    //         xy1[k] = node1->xyz_coords[k];
    //     }
    //
    //
    //     area = 0.0;
    //     xy_new_center[0] = 0.0;
    //     xy_new_center[1] = 0.0;
    //     xy_new_center[2] = 0.0;
    //
    //     for (j=0; j<node1->friend_num; j++)
    //     {
    //         node2 = node1->friends_list[j];
    //         node3 = node1->friends_list[(j+1)%node1->friend_num];
    //
    //         for (k=0; k<3; k++)
    //         {
    //             sph2[k] = node1->centroids[j][k];
    //             sph3[k] = node1->centroids[(j+1)%node1->friend_num][k];
    //         }
    //
    //         sph1[1] = pi*0.5 - sph1[1];
    //         sph2[1] = pi*0.5 - sph2[1];
    //         sph3[1] = pi*0.5 - sph3[1];
    //
    //         sph2cart(sph2, xy2);
    //         sph2cart(sph3, xy3);
    //
    //         for (k=0; k<3; k++) xy_sub_centers[j][k] = (xy1[k]+xy2[k]+xy3[k])/3.0;
    //
    //         sph1[1] = pi*0.5 - sph1[1];
    //         sph2[1] = pi*0.5 - sph2[1];
    //         sph3[1] = pi*0.5 - sph3[1];
    //
    //         areas[j] = sphericalArea(sph1, sph2, sph3);
    //         area += areas[j];
    //
    //     }
    //
    //     for (j=0; j<node1->friend_num; j++)
    //     {
    //         for (k=0; k<3; k++)
    //         {
    //             xy_new_center[k] += xy_sub_centers[j][k]*areas[j];
    //         }
    //     }
    //
    //     for (k=0; k<3; k++) xy_new_center[k] /= area;
    //
    //     mag = sqrt(xy_new_center[0]*xy_new_center[0]
    //             +xy_new_center[1]*xy_new_center[1]
    //             +xy_new_center[2]*xy_new_center[2]);
    //
    //     for (k=0; k<3; k++) {
    //         xy_new_center[k] /= mag;
    //         shifted_xyz[i][k] = xy_new_center[k];
    //     }
    // }
    //
    // for (i=0; i<node_list.size(); i++)
    // {
    //     new_xyz[0] = shifted_xyz[i][0];
    //     new_xyz[1] = shifted_xyz[i][1];
    //     new_xyz[2] = shifted_xyz[i][2];
    //
    //     node1 = this->node_list[i];
    //     node1->updateXYZ(new_xyz);
    // }

}

void Grid::defineRegion(int reg, int subReg, int ID[])
{
    for (int i=0; i < 3; i++) regions[reg][subReg][i] = ID[i]; //this->node_list[ID[i]];
}

void Grid::findCentroids(void)
{
    int i, j, k;
    double mag;
    Node * node, *node_friend, *node_friend2;
    double xy_center[3];
    double sph_center[3];

    for (i=0; i<node_list.size(); i++)
    {
        node = this->node_list[i];
        for (j=0; j<node->friend_num; j++)
        {
            node_friend  = node->friends_list[j];
            node_friend2 = node->friends_list[(j+1)%node->friend_num];

            for (k=0; k<3; k++) {
                xy_center[k] = (node->xyz_coords[k]
                                + node_friend->xyz_coords[k]
                                + node_friend2->xyz_coords[k]);
            }

            mag = sqrt(xy_center[0]*xy_center[0] + xy_center[1]*xy_center[1] + xy_center[2]*xy_center[2]);
            xy_center[0] /= mag;
            xy_center[1] /= mag;
            xy_center[2] /= mag;

            cart2sph(xy_center, sph_center);

            node->centroids[j][0] = sph_center[0];
            node->centroids[j][1] = sph_center[1];
            node->centroids[j][2] = sph_center[2];

        }

        if (node->friend_num == 5)
        {
            node->centroids[5][0] = -1.0;
            node->centroids[5][1] = -1.0;
            node->centroids[5][2] = -1.0;
        }
    }
}

void Grid::orderNodesByRegion(void)
{
    int i, j, k, f, current_region, current_subregion, node_num, current_node_ID;
    int regIDs[3];
    double v1[3], v2[3], v3[3], v[3];
    Node * current_node;
    Node * p1, * p2, * p3, * p;
    bool inside, onEdge;
    std::vector<int> potential_IDs;
    std::vector<int> immediate_fs(pow(2, this->recursion_lvl) - 1);

    node_num = this->node_list.size();

    std::vector<int> ordered_IDs (node_num);
    std::vector<Node *> ordered_nodes (node_num);
    std::vector<int> not_updated_IDs (this->node_list.size() - 12);
    std::vector<int> updated_IDs;

    // Add pentagons to respective regions
    inside_region[0].push_back(0);
    inside_region[0].push_back(1);

    for (i=0; i < node_num - 12; i++)
    {
        not_updated_IDs[i] = i + 12;
    }

    int top_node_ID;
    int node_f_ID;
    int row_num;
    int f_num;
    int x, y;
    Node * top_node_reg, * top_node_row, * node_f, * previous_node;


    for (i=0; i<5; i++)
    {
      // Find the first neighbour to the pole on edge 1
      for (j = 0; j<5; j++)
      {
        node_f = node_list[0]->friends_list[j];

        p1 = node_list[ regions[i][0][0] ];
        p2 = node_list[ regions[i][0][1] ];

        for (k = 0; k < 3; k++)
        {
          v1[k] = p1->xyz_coords[k];
          v2[k] = p2->xyz_coords[k];
          v[k]  = node_f->xyz_coords[k];
        }

        if (isOnEdge(v1, v2, v))
        {
          top_node_reg = node_f;
          inside_region[i].push_back(node_f->ID);

          if (i==1) std::cout<<top_node_reg->xyz_coords[0]<<'\t'<<top_node_reg->xyz_coords[1]<<'\t'<<top_node_reg->xyz_coords[2]<<std::endl;
          break;
        }
      }

      // Loop through first row using anticlockwise friends until dlon vector changes sign
      top_node_row = top_node_reg;
      current_node = top_node_row;
      previous_node = current_node;

      double dlon = -1.0;
      int nrow = 0;
      while (nrow < pow(2, this->recursion_lvl) - 1)
      {
        p1 = node_list[ regions[i][0][0] ];
        p2 = node_list[ regions[i][0][1] ];
        p3 = node_list[ regions[i][0][2] ];
        for (k = 0; k < 3; k++)
        {
          v1[k] = p1->xyz_coords[k];
          v2[k] = p2->xyz_coords[k];
          v3[k] = p3->xyz_coords[k];
        }

        f_num = current_node->friend_num;

        for (j = 0; j < f_num; j++)
        {
          node_f = current_node->friends_list[j];
          for (k = 0; k < 3; k++) v[k]  = node_f->xyz_coords[k];

          if (isInsideSphericalTriangle(v1, v2, v3, v) && !isOnEdge(v3, v1, v))
          {
            inside_region[i].push_back(node_f->ID);
            immediate_fs[nrow] = j;
            current_node = node_f;
            nrow++;

            if (i==1) std::cout<<current_node->xyz_coords[0]<<'\t'<<current_node->xyz_coords[1]<<'\t'<<current_node->xyz_coords[2]<<std::endl;
            break;
          }
        }
      }



      Node * f_node2;
      int extra;
      bool added;

      for (y=0; y<(nrow+1)*2 - 1; y++)
      {
          for (x=0; x<nrow; x++)
          {
              extra = 0;
              if (i == 0) extra = 2;

              f_num = node_list[ inside_region[i][y*(nrow+1) + x + extra] ]->friend_num;
              current_node = node_list[ inside_region[i][y*(nrow+1) + x + extra] ]->friends_list[ immediate_fs[x] + f_num - 1];

              if (x == 0)
              {
                  added = false;
                  int count = f_num-1;
                  while (!added)
                  {
                      current_node = top_node_row;
                      node_f = current_node->friends_list[count];
                      for (k = 0; k < 3; k++) v[k]  = node_f->xyz_coords[k];

                      for (j=0; j < 4; j++)
                      {
                          p1 = node_list[ regions[i][j][0] ];
                          p2 = node_list[ regions[i][j][1] ];
                          p3 = node_list[ regions[i][j][2] ];
                          for (k = 0; k < 3; k++)
                          {
                              v1[k] = p1->xyz_coords[k];
                              v2[k] = p2->xyz_coords[k];
                              v3[k] = p3->xyz_coords[k];
                          }

                          if (isInsideSphericalTriangle(v1,v2,v3,v) &&
                              isOnEdge(v1, v2, v) &&
                              node_f->ID != inside_region[i][(y-1)*(nrow+1) + extra])
                          {
                              added = true;
                              current_node = node_f;
                              break;
                          }
                      }
                      count--;
                  }

                  top_node_row = current_node;
              }

              if (i==1) std::cout<<current_node->xyz_coords[0]<<'\t'<<current_node->xyz_coords[1]<<'\t'<<current_node->xyz_coords[2]<<std::endl;

              inside_region[i].push_back(current_node->ID);
          }

          f_node2 = node_list[ inside_region[i][y*(nrow+1) + x + extra] ];

          for (j=0; j<6; j++)
          {
              node_f = current_node->friends_list[j];

              if (current_node->friends_list[(j+1)%6] == f_node2)
              {
                  if (i==1) std::cout<<node_f->xyz_coords[0]<<'\t'<<node_f->xyz_coords[1]<<'\t'<<node_f->xyz_coords[2]<<std::endl;
                  inside_region[i].push_back(node_f->ID);
                  break;
              }

          }

      }

      // std::cout<<inside_region[i].size()-2<<", "<<nrow<<std::endl;

        // Return to beginning of row and find first clockwise friend in region

        // Find next clockwise friend that also is friends with node at ordered_list - nrow
    }

    int count = 0;
    for (i=0; i<5; i++)
    {
        for (j=0; j<inside_region[i].size(); j++)
        {
            ordered_IDs[count] = inside_region[i][j];
            count++;
        }
    }

    for (i=0; i<node_num; i++)
    {
        ordered_nodes[i] = node_list[ordered_IDs[i]];
        ordered_nodes[i]->ID = i;
    }

    for (i=0; i<node_num; i++)
    {
        node_list[i] = ordered_nodes[i];
    }

}

void Grid::saveGrid2File(void)
{
    FILE * outFile;
    Node * node;
    double lat, lon;
    int i, j;
    int f[6];
    double cx[6], cy[6];

    outFile = fopen("test_file.txt", "w");

    fprintf(outFile, "%-5s %-12s %-12s %-38s %-20s\n", "ID", "NODE LAT", "NODE LON", "FRIENDS LIST", "CENTROID COORD LIST");


    for (i=0; i<this->node_list.size(); i++)
    {
        node = this->node_list[i];
        lat = node->sph_coords[1]*180./pi;
        lon = node->sph_coords[2]*180./pi + 180.0;

        if (lon>359.99) lon = 0.0;
        if (i<2) lon = 180.0;
        for (j=0; j<node->friend_num; j++)
        {
            f[j] = node->friends_list[j]->ID;
            cx[j] = node->centroids[j][1]*180./pi;
            cy[j] = node->centroids[j][2]*180./pi + 180.0;
            if (cy[j]>359.99) cy[j] =0.0;
        }
        if (node->friend_num == 5) {
            f[5] = -1;
            cx[j] = -1.0;
            cy[j] = -1.0;
        }
        fprintf(outFile, "%-5d %12.16f %12.16f { %4d, %4d, %4d, %4d, %4d, %4d}, {( % 10.16f, % 10.16f), ( % 10.16f, % 10.16f), ( % 10.16f, % 10.16f), ( % 10.16f, % 10.16f), ( % 10.16f, % 10.16f), ( % 10.16f, % 10.16f)} \n",
                i, lat, lon, f[0], f[1], f[2], f[3], f[4], f[5],
                cx[0], cy[0], cx[1], cy[1], cx[2], cy[2], cx[3], cy[3], cx[4], cy[4], cx[5], cy[5]);
    }


    fclose(outFile);
}
