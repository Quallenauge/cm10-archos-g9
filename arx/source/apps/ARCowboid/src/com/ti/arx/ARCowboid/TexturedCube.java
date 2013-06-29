/*
 * Copyright (C) 2011 Texas Instruments Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.ti.arx.ARCowboid;

import java.nio.FloatBuffer;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.lang.Float;

import java.io.IOException;
import java.io.InputStream;

import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLUtils;

import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.opengles.GL11;

public class TexturedCube {

    private FloatBuffer vertices;
    private FloatBuffer texCoords;
    private ByteBuffer indices;

    private int mTextureID;
    private int mVtxVboID;
    private int mTexVboID;
    private int mIndVboID;

    private int mResourceId;
    private Resources mResources;

    public TexturedCube(Resources res, int resourceId) {
        this.mResources = res;
        this.mResourceId = resourceId;

        float cubeVertices[] = {
            //front face
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,

            //right face
            1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,

            //back face
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,

            //left face
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,

            //bottom face
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,

            //top face
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
        };

        float texFaceCoords[] = {
            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
        };

        byte elIndices[] = {
            0,1,3, 0,3,2,
            4,5,7, 4,7,6,
            8,9,11, 8,11,10,
            12,13,15, 12,15,14,
            16,17,19, 16,19,18,
            20,21,23, 20,23,22,
        };

        ByteBuffer byteBuf = ByteBuffer.allocateDirect(cubeVertices.length * Float.SIZE);
        byteBuf.order(ByteOrder.nativeOrder());
        vertices = byteBuf.asFloatBuffer();
        vertices.put(cubeVertices);
        vertices.position(0);

        byteBuf = ByteBuffer.allocateDirect(texFaceCoords.length * 6 * Float.SIZE);
        byteBuf.order(ByteOrder.nativeOrder());
        texCoords = byteBuf.asFloatBuffer();
        for(int i=0;i<6;i++)
            texCoords.put(texFaceCoords);
        texCoords.position(0);

        indices = ByteBuffer.allocateDirect(elIndices.length);
        indices.put(elIndices);
        indices.position(0);
    }

    public void draw(GL10 gl) {
        gl.glActiveTexture(GL10.GL_TEXTURE0);
        gl.glBindTexture(GL10.GL_TEXTURE_2D, mTextureID);
        gl.glEnableClientState(GL10.GL_VERTEX_ARRAY);
        gl.glEnableClientState(GL10.GL_TEXTURE_COORD_ARRAY);
        gl.glFrontFace(GL10.GL_CCW);
        if(gl instanceof GL11) {
            GL11 gl11 = (GL11)gl;
            gl11.glBindBuffer(GL11.GL_ARRAY_BUFFER, mVtxVboID);
            gl11.glVertexPointer(3, GL10.GL_FLOAT, 0, 0);

            gl11.glBindBuffer(GL11.GL_ARRAY_BUFFER, mTexVboID);
            gl11.glTexCoordPointer(2, GL10.GL_FLOAT, 0, 0);
            gl11.glBindBuffer(GL11.GL_ELEMENT_ARRAY_BUFFER, mIndVboID);
            
            gl11.glDrawElements(GL10.GL_TRIANGLES, indices.limit(), GL10.GL_UNSIGNED_BYTE, 0);
            
            gl.glDisableClientState(GL10.GL_VERTEX_ARRAY);
            gl.glDisableClientState(GL10.GL_TEXTURE_COORD_ARRAY);
            
            gl11.glBindBuffer(GL11.GL_ARRAY_BUFFER, 0);
            gl11.glBindBuffer(GL11.GL_ELEMENT_ARRAY_BUFFER, 0);
        }        
    }

    public void setup(GL10 gl10) {
        if(gl10 instanceof GL11) {
            GL11 gl = (GL11)gl10;
            int[] ids = new int[3];
            gl.glGenBuffers(3, ids, 0);
            mVtxVboID = ids[0];
            mTexVboID = ids[1];
            mIndVboID= ids[2];

            gl.glBindBuffer(GL11.GL_ARRAY_BUFFER, mVtxVboID);
            gl.glBufferData(GL11.GL_ARRAY_BUFFER, vertices.capacity(), vertices, GL11.GL_STATIC_DRAW);
            gl.glBindBuffer(GL11.GL_ARRAY_BUFFER, 0);

            gl.glBindBuffer(GL11.GL_ARRAY_BUFFER, mTexVboID);
            gl.glBufferData(GL11.GL_ARRAY_BUFFER, texCoords.capacity(), texCoords, GL11.GL_STATIC_DRAW);
            gl.glBindBuffer(GL11.GL_ARRAY_BUFFER, 0);

            gl.glBindBuffer(GL11.GL_ELEMENT_ARRAY_BUFFER, mIndVboID);
            gl.glBufferData(GL11.GL_ELEMENT_ARRAY_BUFFER, indices.capacity(), indices, GL11.GL_STATIC_DRAW);
            gl.glBindBuffer(GL11.GL_ELEMENT_ARRAY_BUFFER, 0);
        }
        textureLoadFromResource(gl10);
    }

    public void textureLoadFromResource(GL10 gl) {
        InputStream is = mResources.openRawResource(mResourceId);
        Bitmap bitmap = null;
        try {
            bitmap = BitmapFactory.decodeStream(is);
        } finally {
            try {
                is.close();
                is = null;
            } catch (IOException e) {
            }
        }

        int[] ids = new int[1];
        gl.glGenTextures(1, ids, 0);
        mTextureID = ids[0];

        gl.glBindTexture(GL10.GL_TEXTURE_2D, mTextureID);

        gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MIN_FILTER, GL10.GL_NEAREST);
        gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_NEAREST);
        gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_WRAP_S, GL10.GL_REPEAT);
        gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_WRAP_T, GL10.GL_REPEAT);
        GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, bitmap, 0);

        bitmap.recycle();
    }
}
