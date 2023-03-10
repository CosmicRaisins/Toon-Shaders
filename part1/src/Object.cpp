#include "Object.hpp"
#include "Error.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>

Object::Object(){
}

Object::~Object(){
    
}

void Object::LoadFromFile(std::string objectFileName, std::string textureFileName, std::string normalFileName){
    std::ifstream file(objectFileName);
    if (!file)
    {
        std::cerr << "Error when loading .obj file: Couldn't open the file." << std::endl;
        return;
    }

    std::string line;
    int index = 0;

    std::vector<std::vector<float>> position;
    std::vector<std::vector<float>> normals;
    std::vector<std::vector<float>> texture;
    std::vector<std::vector<int>> indices;

    while (getline(file, line))
    {
        if (line.substr(0 ,2) == "v " )
        {
            this->parse_vertex(line.substr(2), position);
        }

        if (line.substr(0 ,3) == "vt " )
        {
            this->parse_texture(line.substr(3), index++, texture);
        }

        if (line.substr(0 ,3) == "vn " )
        {
            this->parse_vertex(line.substr(3), normals);
        }

        else if (line.substr(0, 2) == "f ")
        {
            this->parse_index(line.substr(2), indices);
        }
    }

    int counter = 0;
    std::vector<int> tempIndices;
    for (auto & group: indices) {
        m_geometry.AddVertex(position[group[0]][0], position[group[0]][1], position[group[0]][2]);
        m_geometry.AddTexture(texture[group[1]][0], texture[group[1]][1]);
        m_geometry.AddNormal(normals[group[2]][0], normals[group[2]][1], normals[group[2]][2]);
        tempIndices.push_back(counter);
        counter++;
    }

    for (int i = 0; i < tempIndices.size(); i++){
        int a = tempIndices[i];
        ++i;
        int b = tempIndices[i];
        ++i;
        int c = tempIndices[i];
        m_geometry.MakeTriangle(a,b,c);
    }

    m_geometry.Gen();

    // Create a buffer and set the stride of information
    // NOTE: We are leveraging our data structure in order to very cleanly
    //       get information into and out of our data structure.
    m_vertexBufferLayout.CreateNormalBufferLayout(m_geometry.GetBufferDataSize(),
                                                  m_geometry.GetIndicesSize(),
                                                  m_geometry.GetBufferDataPtr(),
                                                  m_geometry.GetIndicesDataPtr());

    // Load our actual texture
    // We are using the input parameter as our texture to load
    m_textureDiffuse.LoadTexture(textureFileName.c_str());

    // Load the normal map texture
    m_normalMap.LoadTexture(normalFileName);


    // Setup shaders
    std::string vertexShader = m_shader.LoadShader("./shaders/vert.glsl");
    std::string fragmentShader = m_shader.LoadShader("./shaders/frag.glsl");
    // Actually create our shader
    m_shader.CreateShader(vertexShader,fragmentShader);

    file.close();
}

void Object::parse_vertex(std::string const& line, std::vector<std::vector<float>>& position)
{
    std::vector<std::string> tokenized = Object::tokenize(line, ' ');
    std::vector<float> vertex(tokenized.size());
    transform(tokenized.begin(), tokenized.begin() + 3, vertex.begin(), [](const std::string& val)
    {
        return std::stof(val);
    });
    position.push_back(vertex);
}

void Object::parse_texture(std::string const& line, int index, std::vector<std::vector<float>>& texture)
{
    std::vector<std::string> tokenized = Object::tokenize(line, ' ');
    std::vector<float> vertex(tokenized.size());
    transform(tokenized.begin(), tokenized.begin() + 2, vertex.begin(), [](const std::string& val)
    {
        return std::stof(val);
    });
    texture.push_back(vertex);
}


void Object::parse_index(std::string const& line, std::vector<std::vector<int>>& indices)
{
    std::vector<std::string> tokenized = Object::tokenize(line, ' ');
    for (int i = 0; i < 3; i++)
    {
        std::vector<int> vert;
        vert.push_back(std::stoul(Object::tokenize(tokenized[i], '/')[0]) - 1);
//        std::cout << std::stoul(Object::tokenize(tokenized[i], '/')[0]) - 1 << std::endl;
        vert.push_back(std::stoul(Object::tokenize(tokenized[i], '/')[1]) - 1);
//        std::cout << std::stoul(Object::tokenize(tokenized[i], '/')[1]) - 1 << std::endl;
        vert.push_back(std::stoul(Object::tokenize(tokenized[i], '/')[2]) - 1);
//        std::cout << std::stoul(Object::tokenize(tokenized[i], '/')[2]) - 1 << std::endl;
        indices.push_back(vert);
    }

}


// TODO: In the future it may be good to 
// think about loading a 'default' texture
// if the user forgets to do this action!
void Object::LoadTexture(std::string fileName){
        // Load our actual textures
        m_textureDiffuse.LoadTexture(fileName);
}

// Bind everything we need in our object
// Generally this is called in update() and render()
// before we do any actual work with our object
void Object::Bind(){
        // Make sure we are updating the correct 'buffers'
        m_vertexBufferLayout.Bind();
        // Diffuse map is 0 by default, but it is good to set it explicitly
        m_textureDiffuse.Bind(0);
        // We need to set the texture slot explicitly for the normal map  
        m_normalMap.Bind(1);
        // Select our appropriate shader
        m_shader.Bind();
}

Shader* Object::getShader(){
    return &this->m_shader;
}

void Object::Update(unsigned int screenWidth, unsigned int screenHeight){
        // Call our helper function to just bind everything
        Bind();
        // TODO: Read and understand
        // For our object, we apply the texture in the following way
        // Note that we set the value to 0, because we have bound
        // our texture to slot 0.
        m_shader.SetUniform1i("u_DiffuseMap",0);
        // If we want to load another texture, we assign it to another slot
        m_shader.SetUniform1i("u_NormalMap",1);  
         // Here we apply the 'view' matrix which creates perspective.
        // The first argument is 'field of view'
        // Then perspective
        // Then the near and far clipping plane.
        // Note I cannot see anything closer than 0.1f units from the screen.
        // TODO: In the future this type of operation would be abstracted away
        //       in a camera class.
        m_projectionMatrix = glm::perspective(glm::radians(45.0f),((float)screenWidth)/((float)screenHeight),0.1f,20.0f);

        // Set the uniforms in our current shader
        m_shader.SetUniformMatrix4fv("modelTransformMatrix",m_transform.GetTransformMatrix());
        m_shader.SetUniformMatrix4fv("projectionMatrix", &m_projectionMatrix[0][0]);

        // Create a first 'light'
        // Set in a light source position
        m_shader.SetUniform3f("lightPos",2.0f,2.4f,-0.6f);
        // Set a view and a vector
        m_shader.SetUniform3f("viewPos",0.0f,0.0f,0.0f);

}

// Render our geometry
void Object::Render(){
    // Call our helper function to just bind everything
    Bind();
	//Render data
    glDrawElements(GL_TRIANGLES,
                   m_geometry.GetIndicesSize(), // The number of indices, not triangles.
                   GL_UNSIGNED_INT,             // Make sure the data type matches
                        nullptr);               // Offset pointer to the data.
                                                // nullptr because we are currently bound
}

// Returns the actual transform stored in our object
// which can then be modified
Transform& Object::GetTransform(){
    return m_transform; 
}

std::vector<std::string> Object::tokenize(const std::string& line, char delim)
{
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string s;
    while (std::getline(ss, s, delim)) {
        result.push_back(s);
    }
    return result;
}
