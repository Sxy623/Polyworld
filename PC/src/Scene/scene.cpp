//
// Created by dydxh on 11/19/18.
//

#include "scene.h"
#include "../Resource/resourcemanager.h"

Scene::Scene(glm::vec3 initpos, Shader shader) : offset(initpos), map_shader(shader) {
    this->Initialize();
}

Scene::~Scene() {

}

void Scene::Initialize() {
    for(GLint i = 0; i < 3; i++){
        this->tmp_chunk[i] = new Chunk(0, 0);
        this->tmp_chunk[i]->parent = this;
    }
    for(GLint i = 0; i < CHUNK_SIZE; i++) {
        for(GLint j = 0; j < CHUNK_SIZE; j++) {
            this->chunk[i][j] = new Chunk(i, j);
            this->chunk[i][j]->parent = this;
        }
    }
    for(GLint i = 0; i < CHUNK_SIZE; i++) {
        for(GLint j = 0; j < CHUNK_SIZE; j++) {
            this->UpdateNeighbor(i, j);
        }
    }

    this->cur_Chunk = this->chunk[CHUNK_RADIUS][CHUNK_RADIUS];
    this->cur_Submesh = this->cur_Chunk->submesh[MESH_RADIUS][MESH_RADIUS];
    this->InitBuffer();

    this->generator = new Noise(NOISE_SIZE);
    this->water = new Water();
    this->plant[0] = new Plants("pine");
    this->plant[1] = new Plants("normaltree");
    this->plant[2] = new Plants("smallrock");

    for(GLint i = 0; i < CHUNK_SIZE; i++){
        for(GLint j = 0; j < CHUNK_SIZE; j++){
            chunk_offset[i][j] = glm::vec3((i - CHUNK_RADIUS) * CHUNK_LENGTH, 0.0f, (j - CHUNK_RADIUS) * CHUNK_LENGTH);
        }
    }
    for(GLint i = 0; i <= MESH_SIZE; i++){
        for(GLint j = 0; j <= MESH_SIZE; j++){
            mesh_offset[i][j] = glm::vec3((i - MESH_RADIUS - 0.5f) * MESH_LENGTH, 0.0f, (j - MESH_RADIUS - 0.5f) * MESH_LENGTH);
        }
    }
}
void Scene::InitBuffer() {
    this->vertices = {
            // Pos
            1.0f, 1.0f,
            1.0f, 0.0f,
            0.0f, 1.0f,

            0.0f, 1.0f,
            1.0f, 0.0f,
            0.0f, 0.0f
    };

    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);

    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    glBindVertexArray(this->VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    GLfloat instance_vertices[] = {
            1.0f, 1.0f, -1.0f,
            1.0f, 0.0f, -1.0f,
            0.0f, 1.0f, -1.0f,

            0.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f
    };
    glGenVertexArrays(1, &this->instanceVAO);
    glGenBuffers(1, &this->instanceVBO);

    glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(instance_vertices), &instance_vertices, GL_STATIC_DRAW);

    glBindVertexArray(this->instanceVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void Scene::generate_cloud(GLfloat dt) {
    for(GLint i = 0; i < CHUNK_SIZE; i++) {
        for (GLint j = 0; j < CHUNK_SIZE; j++) {
            chunk[i][j]->generate_cloud(dt);
        }
    }
}

void Scene::generate_scene() {
    for(GLint i = 0; i < CHUNK_SIZE; i++) {
        for (GLint j = 0; j < CHUNK_SIZE; j++) {
            chunk[i][j]->generate_height();
        }
    }
    generate_cloud(0.0f);
    for(GLint i = 0; i < CHUNK_SIZE; i++) {
        for (GLint j = 0; j < CHUNK_SIZE; j++) {
            chunk[i][j]->Updateinfo();
        }
    }
    std::cout << this->chunk[0][0]->height[0][0] << std::endl;
    glm::vec3 va = glm::vec3(0.0f, 1.10733f, 1.0404f);
    glm::vec3 vb = glm::vec3(1.0404f, -0.552877f, 0.0f);
    glm::vec3 vc = glm::cross(va, vb);
    Tools::PrintVec3(glm::normalize(vc));
//    this->chunk[0][0]->height[1][1] = 3.0f;
}
void Scene::draw(const glm::mat4& view, const glm::mat4& PVMatrix, const glm::mat4& lightSpaceMatrix,
                 const Texture2D& ShadowMap, const Texture2D& BluredShadow) {
    if(ResourceManager::Keys[GLFW_KEY_H]){
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
    }
    map_instance_shader.use();
    glActiveTexture(GL_TEXTURE0);
    this->HeightMap.Bind();
    glActiveTexture(GL_TEXTURE4);
    ShadowMap.Bind();
    glActiveTexture(GL_TEXTURE5);
    BluredShadow.Bind();
    map_instance_shader.setMat4("view", view);
    map_instance_shader.setMat4("PVMatrix", PVMatrix);
    map_instance_shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    map_instance_shader.setVec3("lower_color", LOWER_COLOR);
    map_instance_shader.setVec3("land_color", LAND_COLOR);
    map_instance_shader.setVec3("rock_color", ROCK_COLOR);
    map_instance_shader.setFloat("scalefactor", MESH_LENGTH);
    map_instance_shader.setVec3("scene_offset", this->offset);
    map_instance_shader.setInt("scene_size", MESH_SIZE * CHUNK_SIZE);
    map_instance_shader.setFloat("near_plane", NEAR_PLANE);
    map_instance_shader.setFloat("far_plane", FAR_PLANE);
    map_instance_shader.setLight(ResourceManager::camera.Position);

    glBindVertexArray(this->instanceVAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, MESH_SIZE * MESH_SIZE * CHUNK_SIZE * CHUNK_SIZE);
    glBindVertexArray(0);
    if(ResourceManager::Keys[GLFW_KEY_H]){
        glCullFace(GL_BACK);
    }

#ifdef viewnormal
    Shader normvis = ResourceManager::GetShader("normvis");
    normvis.use();
    glActiveTexture(GL_TEXTURE0);
    this->HeightMap.Bind();
    normvis.setMat4("PVMatrix", PVMatrix);
    normvis.setMat4("projection", glm::perspective(glm::radians(ResourceManager::camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f));
    normvis.setMat4("view", ResourceManager::camera.GetViewMatrix());
    normvis.setFloat("scalefactor", MESH_LENGTH);
    normvis.setVec3("scene_offset", this->offset);
    normvis.setInt("scene_size", MESH_SIZE * CHUNK_SIZE);
    normvis.setVec3("lightdir", PARLIGHT_DIR);
    glBindVertexArray(this->instanceVAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, MESH_SIZE * MESH_SIZE * CHUNK_SIZE * CHUNK_SIZE);
    glBindVertexArray(0);
#endif //viewnormal

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    water_shader.use();
    glActiveTexture(GL_TEXTURE0);
    ShadowMap.Bind();
    glActiveTexture(GL_TEXTURE1);
    BluredShadow.Bind();
    water_shader.setMat4("view", view);
    water_shader.setMat4("PVMatrix", PVMatrix);
    water_shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    water_shader.setFloat("water_height", SEA_LEVEL);
    water_shader.setFloat("scalefactor", MESH_LENGTH);
    water_shader.setInt("scene_size", MESH_SIZE * CHUNK_SIZE);
    water_shader.setVec3("scene_offset", this->offset);
    water_shader.setVec3("water_color", SEA_COLOR);
    water_shader.setLight(ResourceManager::camera.Position);
    this->water->Setwater(water_shader);

    glBindVertexArray(this->VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, MESH_SIZE * MESH_SIZE * CHUNK_SIZE * CHUNK_SIZE);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glActiveTexture(GL_TEXTURE0);
}
void Scene::drawssr(Shader shader, const glm::mat4 &PVMatrix, const glm::mat4 &viewmat,
                    const glm::mat4 &lightSpaceMatrix, const Texture2D &BluredShadow) {
    shader.use();
    glActiveTexture(GL_TEXTURE0);
    this->HeightMap.Bind();
    glActiveTexture(GL_TEXTURE1);
    BluredShadow.Bind();
    shader.setMat4("PVMatrix", PVMatrix);
    shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    shader.setMat4("view", viewmat);
    shader.setVec3("lower_color", LOWER_COLOR);
    shader.setVec3("land_color", LAND_COLOR);
    shader.setVec3("rock_color", ROCK_COLOR);
    shader.setFloat("scalefactor", MESH_LENGTH);
    shader.setVec3("scene_offset", this->offset);
    shader.setInt("scene_size", MESH_SIZE * CHUNK_SIZE);
    shader.setFloat("near_plane", NEAR_PLANE);
    shader.setFloat("far_plane", FAR_PLANE);
    shader.setLight(ResourceManager::camera.Position);

    glBindVertexArray(this->instanceVAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, MESH_SIZE * MESH_SIZE * CHUNK_SIZE * CHUNK_SIZE);
    glBindVertexArray(0);
}

void Scene::Generate_ShadowMap(const glm::mat4& lightSpaceMatrix, const glm::mat4& view) {
    shadow_shader.use();
    glActiveTexture(GL_TEXTURE0);
    this->HeightMap.Bind();
    shadow_shader.setMat4("PVMatrix", lightSpaceMatrix);
    shadow_shader.setMat4("view", view);
    shadow_shader.setFloat("scalefactor", MESH_LENGTH);
    shadow_shader.setVec3("scene_offset", this->offset);
    shadow_shader.setInt("scene_size", MESH_SIZE * CHUNK_SIZE);
    shadow_shader.setVec3("lightdir", PARLIGHT_DIR);
    shadow_shader.setFloat("near_plane", NEAR_PLANE);
    shadow_shader.setFloat("far_plane", FAR_PLANE);

    glBindVertexArray(this->instanceVAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, MESH_SIZE * MESH_SIZE * CHUNK_SIZE * CHUNK_SIZE);
    glBindVertexArray(0);
}

void Scene::UpdateChunks(glm::vec3 Position) {
    this->debugflag = false;
    Direction dir = ORIGIN_POS;
    GLint Chunkx, Chunkz, Meshx, Meshz;
    GetLocationbyCamera(Position, Chunkx, Chunkz, Meshx, Meshz);
//    printf("(%d, %d) -> (%d, %d)\n", Chunkx, Chunkz, Meshx, Meshz);
    if(Chunkx == cur_Chunk->pos_x && Chunkz == cur_Chunk->pos_z){
        cur_Submesh = cur_Chunk->submesh[Meshx][Meshz];
        ResourceManager::dir = ORIGIN_POS;
        return ;
    }
    this->debugflag = false;
    static auto toUpdate = new Chunk*[CHUNK_SIZE];
    static GLint head, tail;
    head = tail = 0;
    if(Chunkx > cur_Chunk->pos_x){
        this->offset += glm::vec3(CHUNK_LENGTH, 0.0f, 0.0f);
        for(GLint i = 0; i < CHUNK_SIZE; i++) {
            Chunk* tmpChunk = this->chunk[0][i];
            for (GLint j = 1; j < CHUNK_SIZE; j++) {
                this->chunk[j - 1][i] = this->chunk[j][i];
                this->chunk[j - 1][i]->pos_x = j - 1;
            }
            this->chunk[CHUNK_SIZE - 1][i] = tmpChunk;
            tmpChunk->recycle(CHUNK_SIZE - 1, i);
            toUpdate[tail++] = tmpChunk;
        }
        dir = MOVE_XPOS;
    }
    else if(Chunkx < cur_Chunk->pos_x){
        this->offset -= glm::vec3(CHUNK_LENGTH, 0.0f, 0.0f);
        for(GLint i = 0; i < CHUNK_SIZE; i++) {
            Chunk* tmpChunk = this->chunk[CHUNK_SIZE - 1][i];
            for (GLint j = CHUNK_SIZE - 1; j > 0; j--){
                this->chunk[j][i] = this->chunk[j - 1][i];
                this->chunk[j][i]->pos_x = j;
            }
            this->chunk[0][i] = tmpChunk;
            tmpChunk->recycle(0, i);
            toUpdate[tail++] = tmpChunk;
        }
        dir = MOVE_XNEG;
    }
    if(Chunkz > cur_Chunk->pos_z){
        this->offset += glm::vec3(0.0f, 0.0f, CHUNK_LENGTH);
        for(GLint i = 0; i < CHUNK_SIZE; i++) {
            Chunk* tmpChunk = this->chunk[i][0];
            for (GLint j = 1; j < CHUNK_SIZE; j++){
                this->chunk[i][j - 1] = this->chunk[i][j];
                this->chunk[i][j - 1]->pos_z = j - 1;
            }
            this->chunk[i][CHUNK_SIZE - 1] = tmpChunk;
            tmpChunk->recycle(i, CHUNK_SIZE - 1);
            toUpdate[tail++] = tmpChunk;
        }
        dir = MOVE_ZPOS;
    }
    else if(Chunkz < cur_Chunk->pos_z){
        this->offset -= glm::vec3(0.0f, 0.0f, CHUNK_LENGTH);
        for(GLint i = 0; i < CHUNK_SIZE; i++) {
            Chunk* tmpChunk = this->chunk[i][CHUNK_SIZE - 1];
            for (GLint j = CHUNK_SIZE - 1; j > 0; j--){
                this->chunk[i][j] = this->chunk[i][j - 1];
                this->chunk[i][j]->pos_z = j;
            }
            this->chunk[i][0] = tmpChunk;
            tmpChunk->recycle(i, 0);
            toUpdate[tail++] = tmpChunk;
        }
        dir = MOVE_ZNEG;
    }
    for(GLint i = 0; i < CHUNK_SIZE; i++) {
        this->UpdateNeighbor(0, i);
        this->UpdateNeighbor(1, i);
        this->UpdateNeighbor(i, 0);
        this->UpdateNeighbor(i, 1);
        this->UpdateNeighbor(CHUNK_SIZE - 1, i);
        this->UpdateNeighbor(CHUNK_SIZE - 2, i);
        this->UpdateNeighbor(i, CHUNK_SIZE - 1);
        this->UpdateNeighbor(i, CHUNK_SIZE - 2);
    }
    while(head < tail){
        toUpdate[head++]->Updateinfo();
    }
    cur_Chunk = this->chunk[CHUNK_RADIUS][CHUNK_RADIUS];
    cur_Submesh = cur_Chunk->submesh[MESH_RADIUS][MESH_RADIUS];

    ResourceManager::dir = dir;
}
void Scene::UpdateNeighbor(GLint x, GLint y) {
    if(x > 0) chunk[x][y]->xNeg = chunk[x - 1][y];
    else chunk[x][y]->xNeg = nullptr;
    if(y > 0) chunk[x][y]->zNeg = chunk[x][y - 1];
    else chunk[x][y]->zNeg = nullptr;
    if(x < CHUNK_SIZE - 1) chunk[x][y]->xPos = chunk[x + 1][y];
    else chunk[x][y]->xPos = nullptr;
    if(y < CHUNK_SIZE - 1) chunk[x][y]->zPos = chunk[x][y + 1];
    else chunk[x][y]->zPos = nullptr;
}

void Scene::GetLocationbyCamera(glm::vec3 Position, GLint &cx, GLint &cz, GLint &mx, GLint &mz) {
    glm::vec3 position = Position -
                         chunk[CHUNK_RADIUS][CHUNK_RADIUS]->submesh[MESH_RADIUS][MESH_RADIUS]->get_Position() -
                         glm::vec3(MESH_LENGTH / 2.0f, 0.0f, MESH_LENGTH / 2.0f);
//    position = glm::vec3(0.0f) -
//               chunk[CHUNK_RADIUS][CHUNK_RADIUS]->submesh[MESH_RADIUS][MESH_RADIUS]->get_Position() -
//               glm::vec3(MESH_LENGTH / 2.0f, 0.0f, MESH_LENGTH / 2.0f);
//    printf("camera position = ");
//    Tools::PrintVec3(ResourceManager::camera.Position);
//    printf("center position = ");
//    Tools::PrintVec3(chunk[CHUNK_RADIUS][CHUNK_RADIUS]->submesh[MESH_RADIUS][MESH_RADIUS]->get_Position() + glm::vec3(MESH_LENGTH / 2.0f, 0.0f, MESH_LENGTH / 2.0f));

    if(position.x > 0)
        cx = static_cast<GLint>(position.x / CHUNK_LENGTH + 0.5f) + CHUNK_RADIUS;
    else
        cx = static_cast<GLint>(position.x / CHUNK_LENGTH - 0.5f) + CHUNK_RADIUS;
    if(position.z > 0)
        cz = static_cast<GLint>(position.z / CHUNK_LENGTH + 0.5f) + CHUNK_RADIUS;
    else
        cz = static_cast<GLint>(position.z / CHUNK_LENGTH - 0.5f) + CHUNK_RADIUS;
    position.x -= (cx - CHUNK_RADIUS) * CHUNK_LENGTH;
    position.z -= (cz - CHUNK_RADIUS) * CHUNK_LENGTH;
    if(position.x > 0)
        mx = static_cast<GLint>(position.x / MESH_LENGTH + 0.5f) + MESH_RADIUS;
    else
        mx = static_cast<GLint>(position.x / MESH_LENGTH - 0.5f) + MESH_RADIUS;
    if(position.z > 0)
        mz = static_cast<GLint>(position.z / MESH_LENGTH + 0.5f) + MESH_RADIUS;
    else
        mz = static_cast<GLint>(position.z / MESH_LENGTH - 0.5f) + MESH_RADIUS;
}
bool Scene::ValidPlace(int cx, int cz, int mx, int mz){
    static int Radius = 2;
    for(int i = -Radius; i <= Radius; i++){
        for(int j = -Radius; j <= Radius; j++){
            int tx = cx, tz = cz, hx = mx + i, hz = mz + j;
            if(hx < 0) tx--, hx = MESH_SIZE - hx;
            else if(hx >= MESH_SIZE) tx++, hx += MESH_SIZE;
            if(hz < 0) tz--, hz = MESH_SIZE - hz;
            else if(hz >= MESH_SIZE) tz++, hz += MESH_SIZE;
            if(chunk[tx][tz]->height[hx][hz] >= 0.1) return false;
        }
    }
    return true;
}
glm::vec3 Scene::FindStartFinishLocation(int type) {
    if(type == 0){
        for(int k = 0; k < MESH_SIZE; k++)
            for(int h = 0; h < MESH_SIZE; h++){
                int cx = CHUNK_RADIUS, cz = CHUNK_RADIUS;
                if(ValidPlace(cx, cz, k, h))
                    return this->offset + this->chunk_offset[cx][cz] + this->mesh_offset[k][h];
            }
        printf("Can't Generate Start Platform.\n");
        return glm::vec3(100.0f);
    }
    for(int i = 1; i < CHUNK_SIZE - 1; i++)
        for(int j = 1; j < CHUNK_SIZE - 1; j++)
            for(int k = 0; k < MESH_SIZE; k++)
                for(int h = 0; h < MESH_SIZE; h++){
                    int cx = type ? CHUNK_SIZE - i - 1 : i;
                    int cz = type ? CHUNK_SIZE - j - 1 : j;
                    if(ValidPlace(cx, cz, k, h))
                        return this->offset + this->chunk_offset[cx][cz] + this->mesh_offset[k][h];
                }
}

int Scene::PlaceEnable(int i, int j, int k, int h, bool insea){
    static int Radius[] = {3, 4, 2};
    const int maxRadius = 4;

    GLfloat height = this->chunk[i][j]->height[k][h];
    bool flag[] = {!insea, !insea, !insea};
    for(int a = 0; a < maxRadius; a++){
        for(int b = 0; b < maxRadius; b++){
            if(a == 0 && b == 0) continue;
            int curRadius = glm::max(a, b);
            GLfloat tmpheight;
            if(k >= a && h >= b)
                tmpheight = this->chunk[i][j]->height[k - a][h - b];
            else if(k < a && h < b)
                tmpheight = this->chunk[i - 1][j - 1]->height[MESH_SIZE + k - a][MESH_SIZE + h - b];
            else if(k < a)
                tmpheight = this->chunk[i - 1][j]->height[MESH_SIZE + k - a][h - b];
            else
                tmpheight = this->chunk[i][j - 1]->height[k - a][MESH_SIZE + h - b];
            if(height < tmpheight && Radius[0] > curRadius)
                flag[0] = false;
            if(height > tmpheight && Radius[1] > curRadius)
                flag[1] = false;
            if(height > tmpheight && Radius[2] > curRadius)
                flag[2] = false;
            if(k + a < MESH_SIZE && h + b < MESH_SIZE)
                tmpheight = this->chunk[i][j]->height[k + a][h + b];
            else if(k + a >= MESH_SIZE && h + b >= MESH_SIZE)
                tmpheight = this->chunk[i + 1][j + 1]->height[k + a - MESH_SIZE][h + b - MESH_SIZE];
            else if(k + a >= MESH_SIZE)
                tmpheight = this->chunk[i + 1][j]->height[k + a - MESH_SIZE][h + b];
            else
                tmpheight = this->chunk[i][j + 1]->height[k + a][h + b - MESH_SIZE];
            if(height < tmpheight && Radius[0] > curRadius)
                flag[0] = false;
            if(height < tmpheight && Radius[1] > curRadius)
                flag[1] = false;
            if(height > tmpheight && Radius[2] > curRadius)
                flag[2] = false;
        }
    }
    for(int i = 0; i < TREENUMBER; i++)
        if(flag[i])
            return i;
    return -1;
}
void Scene::Generate_Treeplace() {
//    GLuint length = MESH_SIZE * CHUNK_SIZE;
    for(int i = 0; i < TREENUMBER; i++)
        Treeplace[i].clear();
    static Trunk tmptrunk0 = this->plant[0]->modelptr->GetTrunk(1);
    static Trunk tmptrunk1 = this->plant[1]->modelptr->GetTrunk(1);
    static BoundBox tmpbox2 = this->plant[2]->modelptr->GetBoundBox();
    for(int i = 1; i < CHUNK_SIZE - 1; i++)
        for(int j = 1; j < CHUNK_SIZE - 1; j++)
            for(int k = 1; k < MESH_SIZE - 1; k++)
                for(int h = 1; h < MESH_SIZE - 1; h++){
                    if(this->chunk[i][j]->height[k][h] > 1.0f)
                        continue;
                    bool Insea = this->chunk[i][j]->height[k][h] < 0.3f;

                    int TreeType = PlaceEnable(i, j, k, h, Insea);
                    if(TreeType == -1) continue;

                    Treeinfo tmptree;
                    tmptree.type = TreeType < 2 ? 0 : 1;
                    tmptree.chunk_number = i * CHUNK_SIZE + j;
                    tmptree.height = this->chunk[i][j]->height[k][h];
                    tmptree.location = this->chunk[i][j]->submesh[k][h]->get_Position();
                    tmptree.axis = tmptree.type == 0 ? glm::vec3(0.0, 1.0, 0.0) :
                                   glm::normalize(glm::vec3(int(tmptree.height * 10) % 10 / 10.0,
                                                            int(tmptree.height * 100) % 10 / 10.0,
                                                            int(tmptree.height * 1000) % 10 / 10.0));
                    tmptree.angle = int(tmptree.height * 10000.0) % 100 / 50.0 * PI;
                    switch(TreeType){
                        case(0):{
                            tmptree.trunk = tmptrunk0;
                            tmptree.trunk.bottom += plant[0]->modelptr->BiasVector() +
                                                    tmptree.location +
                                                    glm::vec3(0.0f, tmptree.height, 0.0f);
                            tmptree.trunk.height *= 0.3f;
                            tmptree.trunk.Radius *= 0.3f;
                            break;
                        }
                        case(1):{
                            tmptree.trunk = tmptrunk1;
                            tmptree.trunk.bottom += plant[1]->modelptr->BiasVector() +
                                                    tmptree.location +
                                                    glm::vec3(0.0f, tmptree.height, 0.0f);
                            tmptree.trunk.height *= 0.3f;
                            tmptree.trunk.Radius *= 0.3f;
                            break;
                        }
                        case(2):{
                            tmptree.box = tmpbox2;
                            tmptree.box.Point *= glm::vec3(0.05);
                            tmptree.box.Point = plant[2]->modelptr->BiasVector() +
                                             tmptree.location +
                                             glm::vec3(0.0f, tmptree.height, 0.0f);
                            tmptree.box.Front = 1.2f;
                            tmptree.box.Left  = 1.2f;
                            tmptree.box.Down  = 1.2f;
                            break;
                        }
                        default: break;
                    }
                    Treeplace[TreeType].push_back(tmptree);
                }
}

Texture2D Scene::Generate_HeightMap() {
    GLint p = 0;
    GLint limit = CHUNK_SIZE * CHUNK_SIZE * MESH_SIZE * MESH_SIZE + CHUNK_SIZE * MESH_SIZE * 2 + 1;
    GLuint len = CHUNK_SIZE * MESH_SIZE + 1;
    static GLfloat* data = new GLfloat[limit];
    for(GLint i = 0; i < CHUNK_SIZE; i++){
        for(GLint j = 0; j < MESH_SIZE; j++){
            for(GLint k = 0; k < CHUNK_SIZE; k++){
                if(this->debugflag) {
                    for (GLint h = 0; h < MESH_SIZE; h++) {
                        printf("%.3lf ", chunk[i][k]->height[j][h]);
                    }
                    if(k == CHUNK_SIZE - 1){
                        printf("%.3lf", chunk[i][k]->height[j][MESH_SIZE]);
                    }
                }
                memcpy(data + p, chunk[i][k]->height[j], sizeof(GLfloat) * (MESH_SIZE + 1));
                p += MESH_SIZE;
            }
            p++;
            if(this->debugflag) {
                printf("\n");
            }
        }
    }
    for(GLint i = 0; i < CHUNK_SIZE; i++){
        memcpy(data + p, chunk[CHUNK_SIZE - 1][i]->height[MESH_SIZE], sizeof(GLfloat) * (MESH_SIZE + 1));
        p += MESH_SIZE;
    }
    p++;
    for(GLint i = 0; i < limit; i++){
        data[i] = data[i] * 0.1f;
    }
    return ResourceManager::MakeTexture(len, len, GL_RED, data, "HeightMap");
}
Texture2D Scene::Generate_CloudMap(){
    GLint p = 0;
    GLint limit = CHUNK_SIZE * CHUNK_SIZE * MESH_SIZE * MESH_SIZE + CHUNK_SIZE * MESH_SIZE * 2 + 1;
    GLuint len = CHUNK_SIZE * MESH_SIZE + 1;
    static GLfloat* data = new GLfloat[limit];
    for(GLint i = 0; i < CHUNK_SIZE; i++){
        for(GLint j = 0; j < MESH_SIZE; j++){
            for(GLint k = 0; k < CHUNK_SIZE; k++){
                if(this->debugflag) {
                    for (GLint h = 0; h < MESH_SIZE; h++) {
                        printf("%.3lf ", chunk[i][k]->cloud[j][h]);
                    }
                    if(k == CHUNK_SIZE - 1){
                        printf("%.3lf", chunk[i][k]->cloud[j][MESH_SIZE]);
                    }
                }
                memcpy(data + p, chunk[i][k]->cloud[j], sizeof(GLfloat) * (MESH_SIZE + 1));
                p += MESH_SIZE;
            }
            p++;
            if(this->debugflag) {
                printf("\n");
            }
        }
    }
    for(GLint i = 0; i < CHUNK_SIZE; i++){
        memcpy(data + p, chunk[CHUNK_SIZE - 1][i]->cloud[MESH_SIZE], sizeof(GLfloat) * (MESH_SIZE + 1));
        p += MESH_SIZE;
    }
    p++;
    return ResourceManager::MakeTexture(len, len, GL_RED, data, "Could", GL_LINEAR, GL_LINEAR);
}
Texture2D Scene::Generate_NormalMap(int th) {
    GLint p = 0;
    GLint limit = CHUNK_SIZE * CHUNK_SIZE * MESH_SIZE * MESH_SIZE;
    GLuint len = CHUNK_SIZE * MESH_SIZE;
    static glm::vec3* data = new glm::vec3[limit];
    for(GLint i = 0; i < CHUNK_SIZE; i++){
        for(GLint j = 0; j < MESH_SIZE; j++){
            for(GLint k = 0; k < CHUNK_SIZE; k++){
                memcpy(data + p, chunk[i][k]->normal[th][j], sizeof(glm::vec3) * (MESH_SIZE));
                p += MESH_SIZE;
            }
        }
    }
    for(GLint i = 0; i < limit; i++){
        data[i] = data[i] * 0.5f + 0.5f;
    }
    return ResourceManager::MakeTexture(len, len, GL_RGB, data, "NormalMap" + std::to_string(th));
}

Texture2D Scene::Generate_pNormalMap() {
    GLint p = 0;
    GLint limit = CHUNK_SIZE * CHUNK_SIZE * MESH_SIZE * MESH_SIZE + CHUNK_SIZE * MESH_SIZE * 2 + 1;
    GLuint len = CHUNK_SIZE * MESH_SIZE + 1;
    static auto data = new glm::vec3[limit];
    for(GLint i = 0; i < CHUNK_SIZE; i++){
        for(GLint j = 0; j < MESH_SIZE; j++){
            for(GLint k = 0; k < CHUNK_SIZE; k++){
                memcpy(data + p, chunk[i][k]->pnormal[j], sizeof(glm::vec3) * MESH_SIZE);
                p += MESH_SIZE;
            }
            data[p++] = glm::vec3(0.0f);
        }
    }
    for(GLint i = 0; i < CHUNK_SIZE; i++){
        memset(data + p, 0, sizeof(glm::vec3) * MESH_SIZE);
        p += MESH_SIZE;
    }
    data[p++] = glm::vec3(0.0f);
    for(GLint i = 0; i < limit; i++){
        data[i] = data[i] * 0.5f + 0.5f;
    }
    return ResourceManager::MakeTexture(len, len, GL_RGB, data, "pNormalMap");
}