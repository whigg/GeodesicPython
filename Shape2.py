
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.basemap import Basemap
from scipy.stats import mode
import sys

class Shape:
    def __init__(self):
        self.vertex_list = []
        self.friends = []
        self.vertexs = []
        self.factor = 1.2
        self.min_mag = 2.0

    def add_vertex(self, vertex):
        self.vertex_list.append(vertex)

    def bisect_edges(self):
        #print("OLD VERTEX LIST", self.vertex_list)
        vertii = [i for i in self.vertex_list]
        for i in range(len(self.vertex_list)):
            p1 = self.vertex_list[i]
            for j in range(6):
                if self.friends[i][j] >= 0:
                    p2 = self.vertex_list[int(self.friends[i][j])]
                    # if ( p1[0] != p2[0] ) and ( p1[1] != p2[1] ) and ( p1[2] != p2[2] ):
                    vector = []
                    for k in range(3):
                        vector.append((p2[k]+p1[k])*0.5)
                    vector = np.array(vector)

                    vertii.append(vector)

        self.vertex_list = np.array(vertii)

        ncols = self.vertex_list.shape[1]

        dtype = self.vertex_list.dtype.descr * ncols

        struct = self.vertex_list.view(dtype)

        uniq = np.unique(struct)

        self.vertex_list = uniq.view(self.vertex_list.dtype).reshape(-1,ncols)





    def get_min_mag(self):
        mags = []
        p1 = self.vertex_list[0]
        for i in range(len(self.vertex_list[1:])):
            p2 = self.vertex_list[i]

            if (p1[0] != p2[0]) and (p1[0] != p2[0]) and (p1[0] != p2[0]):
                mags.append(np.sqrt(sum((p1-p2)**2)))
                # mags[-1] = np.sqrt(mags[-1])
        self.min_mag = min(mags)
        print("MAX: ", self.min_mag)

    def find_friends(self,level=1.2):
        self.get_min_mag()
        paired = []
        counti = np.zeros(len(self.vertex_list))
        self.friends = np.ones((len(self.vertex_list), 6))*-1
        for i in range(len(self.vertex_list)):
            p1 = self.vertex_list[i]
            #if i%100 == 0:
            #    print(float(i)/float(len(self.vertex_list))*100)
            for j in range(len(self.vertex_list)):
                p2 = self.vertex_list[j]
                mag = np.sqrt(sum((p1-p2)**2.0))
                if (mag <= self.min_mag*level and mag > 0.0):
                    self.friends[i][counti[i]] = j
                    paired.append((i,j))
                    counti[i] += 1

    def scale_vertex(self, scale_factor):
        for i in range(len(self.vertex_list)):
            mag = np.sqrt(sum(self.vertex_list[i]**2.0))
            if (mag < 1.0):
                self.vertex_list[i] *= scale_factor
                ds = abs(1.0 - np.sqrt(sum(self.vertex_list[i]**2.0)))
                while ds > 1e-12:
                    self.vertex_list[i] *= (ds + 1.0)
                    ds = abs(1.0 - np.sqrt(sum(self.vertex_list[i]**2.0)))

    def rotation_matrix(self, axis, theta):
        """
        Return the rotation matrix associated with counterclockwise rotation about
        the given axis by theta radians.
        """
        axis = np.asarray(axis)
        axis = axis/np.sqrt(np.dot(axis, axis))
        a = np.cos(theta/2.0)
        b, c, d = -axis*np.sin(theta/2.0)
        aa, bb, cc, dd = a*a, b*b, c*c, d*d
        bc, ad, ac, ab, bd, cd = b*c, a*d, a*c, a*b, b*d, c*d
        return np.array([[aa+bb-cc-dd, 2*(bc+ad), 2*(bd-ac)],
                         [2*(bc-ad), aa+cc-bb-dd, 2*(cd+ab)],
                         [2*(bd+ac), 2*(cd-ab), aa+dd-bb-cc]])


    def twist_grid(self, angle=np.pi/5.0):
        axis = np.array([0.0, 0.0, 1.0])
        theta = angle

        for i in range(len(self.vertex_list)):
            if self.vertex_list[i][2] < 0.0:
                self.vertex_list[i] = np.dot(self.rotation_matrix(axis,theta), self.vertex_list[i])
            # print(np.dot(rotation_matrix(axis,theta), icosahedron.vertex_list[i]))

    def order_friends(self):
        # for i in range(len(self.vertex_list)):
        #     #Convert central point to spherical coords
        #     point1 = self.cart2sph(self.vertex_list[i])[1:]
        #
        #     #Identify if central point is hexagon or pentagon center
        #     pentagon = 0
        #     if (self.friends[i][-1] < 0.0):
        #         pentagon = 1
        #
        #     #Populate list of all neighbour longitude values
        #     lons = []
        #     for j in range(len(self.friends[i])-pentagon):
        #         lons.append(self.cart2sph(self.vertex_list[int(self.friends[i][j])])[2])
        #
        #     #Populate list of transfrom array. If no transform is needed, the array is 0
        #     #by default.
        #     transform_ind = np.zeros(6)
        #     if point1[1] < 30.0 and max(lons) > 180:
        #         for j in range(len(self.friends[i])-pentagon):
        #             if (lons[j] > 180.0):
        #                 transform_ind[j] = -(360.0-lons[j])
        #     elif point1[1] > 180.0 and min(lons) < 30.0:
        #         for j in range(len(self.friends[i])-pentagon):
        #             if (lons[j] < 30.0):
        #                 transform_ind[j] = 360.0 + lons[j]
        #
        #
        #     mag_north = 1.0
        #     north_vector = np.array([-1.0, 0.0])
        #
        #     angles = []
        #
        #     for j in range(len(self.friends[i])-pentagon):
        #         point2 = self.cart2sph(self.vertex_list[int(self.friends[i][j])])[1:]
        #         if transform_ind[j] != 0.0:
        #             point2[1] = transform_ind[j]
        #
        #
        #         vector = point2 - point1
        #         dot = np.dot(north_vector,vector)
        #         det = north_vector[0]*vector[1] - north_vector[1]*vector[0]
        #         angle = np.arctan2(det,dot)
        #
        #
        #         angles.append(np.rad2deg(angle)+180.0)
        #
        #
        #
        #     count = 0
        #     temp = []
        #     while count < len(angles):
        #         ind = np.argmin(angles)
        #         temp.append(int(self.friends[i][ind]))
        #         angles[ind] = 1000
        #         count+=1
        #
        #     if pentagon:
        #         temp.append(-1)
        #     self.friends[i] = temp

        for i in range(len(self.vertex_list)):
            #Convert central point to spherical coords
            point1 = self.cart2sph(self.vertex_list[i])[1:]

            print(point1)


    def find_centers(self):
        self.centers = np.zeros((len(self.vertex_list),6,2))
        for i in range(len(self.vertex_list)):
            p1 = self.vertex_list[i]

            total = 6
            if self.friends[i][-1] < 0:
                total = 5

            centers_ = np.ones((6,2))*-1
            for j in range(total):
                p2 = self.vertex_list[int(self.friends[i][j])]
                p3 = self.vertex_list[int(self.friends[i][(j+1)%total])]

                center = (p1 + p2 + p3)/3.0

                mag = np.sqrt(sum(center**2))

                center = center/mag

                center = self.cart2sph(center)[1:]

                centers_[j] = center


            self.centers[i] = centers_

    def haversine(self,lat1,lat2,lon1,lon2):

        A = np.sin(0.5*(lat2-lat1))**2
        B = np.cos(lat1)*np.cos(lat2)*np.sin(0.5*(lon2-lon1))**2

        dangle = 2.0*np.arcsin(np.sqrt(A+B))
        return dangle


    def find_arc_lengths(self):
        def haversine(lat1,lat2,lon1,lon2):

            A = np.sin(0.5*(lat2-lat1))**2
            B = np.cos(lat1)*np.cos(lat2)*np.sin(0.5*(lon2-lon1))**2

            dangle = 2.0*np.arcsin(np.sqrt(A+B))
            return dangle

        self.arc_lengths = np.ones((len(self.vertex_list), 6))*-1

        for i in range(len(self.vertex_list)):
            total = 6
            if self.friends[i][-1] < 0:
                total = 5


            for j in range(total):
                p1 = np.deg2rad(self.centers[i][j])
                p2 = np.deg2rad(self.centers[i][(j+1)%total])

                lat1 = p1[0]
                lat2 = p2[0]

                lon1 = p1[1]
                lon2 = p2[1]



                dangle = self.haversine(lat1,lat2,lon1,lon2)

                self.arc_lengths[i][j] = dangle

            #print(self.arc_lengths[i])

    def find_arc_midpoints(self):
        self.arc_mids = np.ones((len(self.vertex_list),6,2))*-1
        for i in range(len(self.vertex_list)):
            total = 6
            if self.friends[i][-1] < 0:
                total = 5


            for j in range(total):
                p1 = np.deg2rad(self.centers[i][j])
                p2 = np.deg2rad(self.centers[i][(j+1)%total])

                lat1 = p1[0]
                lat2 = p2[0]

                lon1 = p1[1]
                lon2 = p2[1]

                Bx = np.cos(lat2)*np.cos(lon2-lon1)
                By = np.cos(lat2)*np.sin(lon2-lon1)
                latm = np.arctan2(np.sin(lat1) + np.sin(lat2), np.sqrt((np.cos(lat1) + Bx)**2 + By**2))

                lonm = lon1 + np.arctan2(By, np.cos(lat1)+Bx)

                self.arc_mids[i][j] = np.rad2deg(np.array([latm,lonm]))


    def find_normals(self):
        self.normals = np.ones((len(self.vertex_list),6,2))*-1.0
        for i in range(len(self.vertex_list)):
            count = 0

            total = len(self.friends[i])

            if self.friends[i][-1] < 0:
                total = 5

            for j in range(total):
                p1 = np.deg2rad(self.centers[i][j])
                p2 = np.deg2rad(self.centers[i][(j+1)%total])

                lat1 = p1[0]
                lat2 = p2[0]
                lon1 = p1[1]
                lon2 = p2[1]

                dangle = self.arc_lengths[i][j]

                #print(dangle)
                #if i==1 and j==0:
                #    print(np.rad2deg(lat1),np.rad2deg(lon1),np.rad2deg(lat2),np.rad2deg(lon2))
                f = np.array([0.5-1e-4,0.5+1e-4])

                latf = np.zeros(2)
                lonf = np.zeros(2)

                for k in range(2):
                    A = np.sin((1.0-f[k])*dangle)/np.sin(dangle)
                    B = np.sin(f[k]*dangle)/np.sin(dangle)

                    x = A * np.cos(lat1) * np.cos(lon1) + B * np.cos(lat2)*np.cos(lon2)
                    y = A * np.cos(lat1) * np.sin(lon1) + B * np.cos(lat2)*np.sin(lon2)
                    z = A*np.sin(lat1) + B*np.sin(lat2)
                    latf[k] = np.arctan2(z,np.sqrt(x**2.0 + y**2.0))
                    lonf[k] = np.arctan2(y,x)

                mp1 = np.rad2deg(np.array([latf[0],lonf[0]]))
                mp2 = np.rad2deg(np.array([latf[1],lonf[1]]))

                dmp = mp2-mp1

                dlat = dmp[0]
                dlon = dmp[1]

                #if i==1 and j==0:
                #    print(mp1,mp2,dlat,dlon)

                self.normals[i][j] = np.array([dlon,-dlat])/np.sqrt(sum(dmp**2))

            #print(self.normals[i])





    def sph2cart(self,r,theta,phi):
        theta = np.deg2rad(theta)
        phi = np.deg2rad(phi)

        x = r*np.sin(theta)*np.cos(phi)
        y = r*np.sin(theta)*np.sin(phi)
        z = r*np.cos(theta)

        return np.array([x, y, z])



    def cart2sph(self,coords):
        x = coords[0]
        y = coords[1]
        z = coords[2]
        r = np.sqrt(x**2 + y**2 + z**2)
        lat = np.pi*0.5 - np.arccos(z/r)
        lon = np.arctan2(y,x)


        if np.rad2deg(lon)+180.0 > 359.9:
            return np.array([r, np.rad2deg(lat), 0.0])
        return np.array([r, np.rad2deg(lat), np.rad2deg(lon)+180.0])


if __name__== '__main__':

    def cart2sph(x,y,z):
        r = np.sqrt(x**2 + y**2 + z**2)
        lat = np.pi*0.5 - np.arccos(z/r)
        lon = np.arctan2(y,x)


        if np.rad2deg(lon)+180.0 > 359.9:
            return [r, np.rad2deg(lat), 0.0]
        return [r, np.rad2deg(lat), np.rad2deg(lon)+180.0]

    def sph2cart(r,theta,phi):
        theta = np.deg2rad(theta)
        phi = np.deg2rad(phi)

        x = r*np.sin(theta)*np.cos(phi)
        y = r*np.sin(theta)*np.sin(phi)
        z = r*np.cos(theta)

        return np.array([x, y, z])





    import Shape2

    f = (1.0 + np.sqrt(5.0))/2.0

    icosahedron = Shape2.Shape()

    # Define the 12 vertices of an icosahedra

    r = 1.0

    v1 = sph2cart(r, 0.0, 0.)
    v2 = sph2cart(r, 180.0, 0.0)

    v3 = sph2cart(r,90.0-26.57, 36.0)
    v4 = sph2cart(r,90.0-26.57, 36.0*3.0)

    v5 = sph2cart(r,90.0-26.57, 36.0*5.0)
    v6 = sph2cart(r,90.0-26.57, 36.0*7.0)
    v7 = sph2cart(r,90.0-26.57, 36.0*9.0)
    v8 = sph2cart(r,(90.0+26.57), 0.0)

    v9 = sph2cart(r,(90.0+26.57), 36.0*2.0)
    v10 = sph2cart(r,(90.0+26.57), 36.0*4.0)
    v11 = sph2cart(r,(90.0+26.57), 36.0*6.0)
    v12 = sph2cart(r,(90.0+26.57), 36.0*8.0)



    icosahedron.add_vertex(v1)
    icosahedron.add_vertex(v2)
    icosahedron.add_vertex(v3)
    icosahedron.add_vertex(v4)

    icosahedron.add_vertex(v5)
    icosahedron.add_vertex(v6)
    icosahedron.add_vertex(v7)
    icosahedron.add_vertex(v8)

    icosahedron.add_vertex(v9)
    icosahedron.add_vertex(v10)
    icosahedron.add_vertex(v11)
    icosahedron.add_vertex(v12)

    print(icosahedron.vertex_list)

    scale_factor = 1.0 / np.sin(2*np.pi/5) / 2.0

    icosahedron.scale_vertex(scale_factor)

    icosahedron.find_friends()

    L = int(sys.argv[1])

    mins = [2.0, 1.0, 0.5, 0.25, 0.125, 0.0625]
    for i in range(L-1):
        print("Calculating L" + str(i+1))
        icosahedron.bisect_edges()
        print("\tBisection complete...")
        icosahedron.scale_vertex(scale_factor)
        print("\tProjecting onto sphere...")
        # icosahedron.min_mag = mins[i+1]
        print("\tIdentifying neighbourghs...\n\n")
        if (i == L-2):
            print("\tRotating Southern Hemisphere by pi/2...")
            icosahedron.twist_grid()
        icosahedron.find_friends()

    print("Ordering neighbouring point in clockwise fashion...")
    icosahedron.order_friends()

    print("Finding triangular centroids...")
    icosahedron.find_centers()


    print("Finding arc lengths between centroids...")
    icosahedron.find_arc_lengths()

    print("Finding arc length centers...")
    icosahedron.find_arc_midpoints()

    print("Finding normal vectors to cells...")
    icosahedron.find_normals()


    print("Calculations complete. Plotting...")
    m = Basemap(projection='ortho',lon_0=0,lat_0=0)
    lats = []
    lons = []
    for i in range(len(icosahedron.centers)):
        total = 6
        if icosahedron.centers[i][-1][0] < 0:
            total = 5
        for j in range(total):
            sph = icosahedron.centers[i][j]
            lats.append(sph[0])
            lons.append(sph[1])

    lats = np.array(lats)
    lons = np.array(lons)



    m = Basemap(projection='hammer',lon_0=180)
    m = Basemap(projection='ortho',lon_0=0,lat_0=90)
    x, y = m(lons,lats)
    # m.scatter(x,y,marker='o',s=8,color='k')
    # plt.show()

    lats = []
    lons = []
    for i in range(len(icosahedron.vertex_list)):
       sph = cart2sph(icosahedron.vertex_list[i][0], icosahedron.vertex_list[i][1], icosahedron.vertex_list[i][2])
       lats.append(sph[1])
       lons.append(sph[2])

    lats = np.array(lats)
    lons = np.array(lons)
    # for num in range(len(icosahedron.vertex_list)):
    #    for i in icosahedron.friends[num]:
    #         if i >= 0:
    #            m.drawgreatcircle(lons[num],lats[num],lons[int(i)],lats[int(i)],c='k',lw=0.5)
    #
    #
    #    total = 6
    #    if icosahedron.friends[num][-1] < 0:
    #        total = 5
    #
    #    for i in range(total):
    #        sph1 = icosahedron.centers[num][i]
    #        lat1 = sph1[0]
    #        lon1 = sph1[1]
    #
    #        sph2 = icosahedron.centers[num][(i+1)%total]
    #        lat2 = sph2[0]
    #        lon2 = sph2[1]
    #
    #        m.drawgreatcircle(lon1, lat1, lon2, lat2, c='b', lw=0.4)
    #
    #        #x, y = m(lon,lat)
    #        #m.scatter(x, y, marker='o',s=2,color='k')
    #    for i in range(total):
    #        lat1 = icosahedron.arc_mids[num][i][0]
    #        lon1 = icosahedron.arc_mids[num][i][1]
    #
    #
    #        x, y = m(lon1,lat1)
    #        m.scatter(x, y, marker='o',s=2,color='k')

    #    print(icosahedron.normals[num])

    # plt.show()

    # f = open('grid_l'+str(L)+'_testing.txt','w')
    # for i in range(len(icosahedron.vertex_list)):
    #     f.write('{:<5d} {: >10.6f}   {: >10.6f}   '.format(i, lats[i], lons[i])) # python will convert \n to os.linesep
    #     string = '{'
    #     for j in range(len(icosahedron.friends[0])):
    #        string += '{:3d}'.format(int(icosahedron.friends[i][j]))
    #        if j < len(icosahedron.friends[0]) - 1:
    #            string += ', '
    #        else:
    #            string += '}\n'
    #     f.write(string)
    # f.close() # you can omit in most cases as the destructor will call it


    f = open('grid_l'+str(L)+'.txt','w')
    # f.write("ID     NODE_LAT      NODE_LON | FRIENDS LIST | CENTROID COORD LIST\n")
    f.write('{:<5s} {: <10s}   {: <10s}   {: <36s}  {:20s}'.format("ID", "NODE_LAT", "NODE_LON", "FRIENDS LIST", "CENTROID COORD LIST\n"))
    for i in range(len(icosahedron.vertex_list)):
        f.write('{:<5d} {: >10.6f}   {: >10.6f}   '.format(i, lats[i], lons[i])) # python will convert \n to os.linesep
        string = '{'
        for j in range(len(icosahedron.friends[0])):
           string += '{:4d}'.format(int(icosahedron.friends[i][j]))
           if j < len(icosahedron.friends[0]) - 1:
               string += ', '
           else:
               string += '}, '

        string += '{'
        for j in range(len(icosahedron.centers[0])):
            string += '({:10.6f}, {:10.6f})'.format(icosahedron.centers[i][j][0], icosahedron.centers[i][j][1])
            if j < len(icosahedron.centers[0]) - 1:
                string += ', '
            else:
                string += '} \n'
        # print(string)
        f.write(string)
    f.close() # you can omit in most cases as the destructor will call it
