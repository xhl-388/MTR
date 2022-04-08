# My Tiny Renderer


## Brief Introduction
:smile:
This is a project based on (or mostly same with ) [ssloy:tinyrenderer](https://github.com/ssloy/tinyrenderer).

I develop the project by my own (?), but provide the same interface as ssloy's project

It is a tiny renderer,which renders object in old way - rasterization.

## How is the project now ?
I roughly implement all the effect in the wiki of ssloy's project , which includes normal mapping ( world or tangent space ),shadow mapping (hard shadow) , Phong shading model , ambient occlusion .

## What's next ?
I want to put some parallel optimization to the code .Actually I have already been using the openmp , but I found in low resolution , the cost of frequently creating new threads is much bigger than the rasterizer itself takes , so I just put it in the AO stage.

Besides the effiency , I want a more clear API system in OpenGL style .I will do that after having a brief review of OpenGL . 

Lastly , more function. Maybe I can use the PBR model.( But I will just do that when I read PBRT) Or add transparent objects.( Too hard ...)