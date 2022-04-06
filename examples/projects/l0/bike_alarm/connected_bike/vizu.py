from pythreejs import *
from stl import mesh

def vizu() :
    # Create a floor
    plane = Mesh(
        PlaneBufferGeometry(100, 100),
        MeshPhysicalMaterial(color='gray'),
        position=[0, -1.5, 0], receiveShadow = True, scale=[1,1,1])
    plane.rotation = (-3.14/2, 0, 0, 'XYZ')

    # Create a camera
    camera = PerspectiveCamera(position=[4, 12, 10], up=[0, 1, 0], aspect=800/400, scale=[1,1,1])

    # Load the STL files and add the vectors to the plot
    geometry = BufferGeometry(attributes={'position': BufferAttribute(array=mesh.Mesh.from_file('bike.stl').vectors)})
    geometry.exec_three_obj_method('computeVertexNormals')

    # Create a robot_mesh to move following our sensor
    bike_mesh = Mesh(
        geometry, MeshLambertMaterial(color='white', dithering= True), position=[-3,-1.5,5],
        castShadow = True, receiveShadow = True, scale=[0.013, 0.013, 0.013])
    bike_mesh.rotation = (-pi/2, 0, pi, 'XYZ')

    # Create a directional ligt folowing our floor
    key_light = SpotLight(scale=[1,1,1], position=[0, 10, 10], angle = 0.8, penumbra = 0.1, target = plane, castShadow = True)
    key_light.shadow.mapSize = (2048, 2048)

    # Create a scene
    scene = Scene(scale=[1,1,1], children=[bike_mesh, plane, camera, key_light, AmbientLight("white", 0.3)])


    # Display the scene with shadow and everything.
    renderer = Renderer(camera=camera, scene=scene,
                        controls=[OrbitControls(controlling=camera)],
                        width=800, height=400,
                        alpha=True,
                        clearOpacity=0,
                       )

    renderer.shadowMap.enabled = True
    renderer.shadowMap.type = 'PCFSoftShadowMap'
    renderer.gammaInput = True;
    renderer.gammaOutput = True;
    display(renderer)
    return bike_mesh
