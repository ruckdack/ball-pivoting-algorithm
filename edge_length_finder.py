import sys
import math

from numpy import Infinity

def csc(x):
    return 1 / math.sin(x)

class ObjLoader(object):
    # __init__ copied from https://inareous.github.io/posts/opening-obj-using-py
    def __init__(self, fileName):
        self.vertices = []
        self.faces = []
        self.edges = []
        self.max_corners = 0
        self.longest_edge = 0
        self.circumscribed_circle_radius = 0
        try:
            f = open(fileName)
            for line in f:
                if line[:2] == 'v ':
                    index1 = line.find(' ') + 1
                    index2 = line.find(' ', index1 + 1)
                    index3 = line.find(' ', index2 + 1)

                    vertex = (float(line[index1:index2]), float(line[index2:index3]), float(line[index3:-1]))
                    vertex = (vertex[0], vertex[1], vertex[2])
                    self.vertices.append(vertex)

                elif line[0] == 'f':
                    string = line.replace('//', '/')
                    ##
                    i = string.find(' ') + 1
                    face = []
                    for item in range(string.count(' ')):
                        if string.find(' ', i) == -1:
                            face.append(int(string[i:-1].split('/')[0])-1)
                            break
                        face.append(int(string[i:string.find(' ', i)].split('/')[0])-1)
                        i = string.find(' ', i) + 1
                    ##
                    self.max_corners = max(len(face), self.max_corners)
                    self.faces.append(tuple(face))

            f.close()
            if len(self.faces) == 0:
                print('no faces found, cannot compute edge length')
                exit()
            self.generate_edges_from_faces()
            self.find_longest_edge()
            self.calc_circumscribed_circle_radius_from_edge()
        except IOError:
            print('.obj file not found.')
    
    def print(self):
        print('vertices: ', self.vertices)
        print()
        print('faces: ', self.faces)

    def generate_edges_from_faces(self):
        for vertex_list in self.faces:
            for i in range(len(vertex_list)):
                self.edges.append((vertex_list[i], vertex_list[(i+1) % len(vertex_list)]))
        
    def find_longest_edge(self):
        current_longest = 0
        for edge in self.edges:
            (a, b) = edge
            v1, v2 = self.vertices[a], self.vertices[b]
            (v1a, v1b, v1c) = v1
            (v2a, v2b, v2c) = v2
            edge_len = math.sqrt((v1a-v2a)**2 + (v1b-v2b)**2 + (v1c-v2c)**2)
            current_longest = max(current_longest, edge_len)
        self.longest_edge = current_longest

    def calc_circumscribed_circle_radius_from_edge(self):
        self.circumscribed_circle_radius = self.longest_edge / 2 * csc(math.pi / self.max_corners)

if len(sys.argv) != 2:
    print('wrong args')
    exit()
obj = ObjLoader(sys.argv[1])
#obj.print()
print('longest edge: ', obj.longest_edge)
print('radius: ', obj.circumscribed_circle_radius)

