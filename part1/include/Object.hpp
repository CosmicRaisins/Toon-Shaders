/** @file Object.hpp
 *  @brief Sets up an OpenGL camera..
 *  
 *  More...
 *
 *  @author Mike
 *  @bug No known bugs.
 */
#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <glad/glad.h>

#include <vector>
#include <string>

#include "Shader.hpp"
#include "VertexBufferLayout.hpp"
#include "Texture.hpp"
#include "Transform.hpp"
#include "Geometry.hpp"

#include "glm/vec3.hpp"
#include "glm/gtc/matrix_transform.hpp"

// Purpose:
// An abstraction to create multiple objects
//
//
class Object{
public:
    // Object Constructor
    Object();
    // Object destructor
    ~Object();
    // Load a texture
    void LoadTexture(std::string fileName);
    // Create a textured quad
    void LoadFromFile(std::string objectFileName, std::string textureFileName, std::string normalFileName);
    // Updates and transformations applied to object
    void Update(unsigned int screenWidth, unsigned int screenHeight);
    // How to draw the object
    void Render();
    // Returns an objects transform
    Transform& GetTransform();
    void parse_index(std::string const& line, std::vector<std::vector<int>>& indices);
    void parse_texture(std::string const& line, int index, std::vector<std::vector<float>>& texture);
    void parse_vertex(std::string const& line, std::vector<std::vector<float>>& position);
    static std::vector<std::string> tokenize(const std::string& line, char delim);
    Shader* getShader();

private:
	// Helper method for when we are ready to draw or update our object
	void Bind();

    // Object vertices
    std::vector<GLfloat> m_vertices;
    // Object indices
    std::vector<GLuint> m_indices;

    // For now we have one shader per object.
    Shader m_shader;
    // For now we have one buffer per object.
    VertexBufferLayout m_vertexBufferLayout;
    // For now we have one texture per object
    Texture m_textureDiffuse;
    // NOTE: that we have a normal map per object as well!
    Texture m_normalMap;
    // Store the objects transformations
    Transform m_transform; 
    // Store the 'camera' projection
    glm::mat4 m_projectionMatrix;
    // Store the objects Geometry
	Geometry m_geometry;
};


#endif
