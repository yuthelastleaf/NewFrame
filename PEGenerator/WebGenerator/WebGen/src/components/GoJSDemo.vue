<template>
    <div ref="graphDiv" style="width: 100%; height: 90%; border: 1px solid black;">
    </div>
    <el-button type="primary" style="height: 5%;" @click="handleClick">点击我</el-button>
</template>

<script setup>
import { onMounted, ref } from 'vue';
import * as joint from 'jointjs';

import { PegenGraph } from '../Graph/PegenGraph.js';

const graphDiv = ref(null);
const pegen_graph = new PegenGraph()

const handleClick = () => {
  console.log('按钮被点击了！');
  pegen_graph.addElement();
};

onMounted(() => {

    pegen_graph.init(graphDiv);
    pegen_graph.addElement();

    // var CustomTextElement = joint.dia.Element.define('examples.CustomTextElement', {
    //     attrs: {
    //         label: {
    //             textAnchor: 'middle',
    //             textVerticalAnchor: 'middle',
    //             fontSize: 48
    //         },
    //         e: {
    //             strokeWidth: 1,
    //             stroke: '#000000',
    //             fill: 'rgba(255,0,0,0.3)'
    //         },
    //         r: {
    //             strokeWidth: 1,
    //             stroke: '#000000',
    //             fill: 'rgba(0,255,0,0.3)'
    //         },
    //         c: {
    //             strokeWidth: 1,
    //             stroke: '#000000',
    //             fill: 'rgba(0,0,255,0.3)'
    //         },
    //         outline: {
    //             ref: 'label',
    //             x: '-calc(0.5*w)',
    //             y: '-calc(0.5*h)',
    //             width: 'calc(w)',
    //             height: 'calc(h)',
    //             strokeWidth: 1,
    //             stroke: '#000000',
    //             strokeDasharray: '5 5',
    //             strokeDashoffset: 2.5,
    //             fill: 'none'
    //         }
    //     }
    // }, {
    //     markup: [{
    //         tagName: 'ellipse',
    //         selector: 'e'
    //     }, {
    //         tagName: 'rect',
    //         selector: 'r'
    //     }, {
    //         tagName: 'circle',
    //         selector: 'c'
    //     }, {
    //         tagName: 'text',
    //         selector: 'label'
    //     }, {
    //         tagName: 'rect',
    //         selector: 'outline'
    //     }]
    // });

    // var boundaryTool = new joint.elementTools.Boundary();
    // var removeButton = new joint.elementTools.Remove();

    // var toolsView = new joint.dia.ToolsView({
    //     tools: [
    //         boundaryTool,
    //         removeButton
    //     ]
    // });

    // var namespace = joint.shapes;
    // namespace.CustomTextElement = CustomTextElement;

    // var graph = new joint.dia.Graph({}, { cellNamespace: namespace });

    // const prect = graphDiv.value.getBoundingClientRect();

    // var paper = new joint.dia.Paper({
    //     el: graphDiv.value,
    //     model: graph,
    //     width: prect.width,
    //     height: prect.height,
    //     gridSize: 10,
    //     drawGrid: true,
    //     background: {
    //         color: 'rgba(0, 255, 0, 0.3)'
    //     },
    //     cellViewNamespace: namespace
    // });

    // var portsIn = {
    //     position: {
    //         name: 'left'
    //     },
    //     attrs: {
    //         portBody: {
    //             magnet: true,
    //             r: 5,
    //             fill: '#023047',
    //             stroke: '#023047'
    //         }
    //     },
    //     label: {
    //         position: {
    //             name: 'left',
    //             args: { y: 6 }
    //         },
    //         markup: [{
    //             tagName: 'text',
    //             selector: 'label',
    //             className: 'label-text'
    //         }]
    //     },
    //     markup: [{
    //         tagName: 'circle',
    //         selector: 'portBody'
    //     }]
    // };

    // var portsOut = {
    //     position: {
    //         name: 'right'
    //     },
    //     attrs: {
    //         portBody: {
    //             magnet: true,
    //             r: 5,
    //             fill: '#E6A502',
    //             stroke: '#023047'
    //         }
    //     },
    //     label: {
    //         position: {
    //             name: 'right',
    //             args: { y: 6 }
    //         },
    //         markup: [{
    //             tagName: 'text',
    //             selector: 'label',
    //             className: 'label-text'
    //         }]
    //     },
    //     markup: [{
    //         // Markup
    //         tagName: 'circle',
    //         selector: 'portBody'
    //     }]
    // };

    // var model = new joint.shapes.standard.Rectangle({
    //     position: { x: 275, y: 50 },
    //     size: { width: 90, height: 90 },
    //     attrs: {
    //         body: {
    //             fill: '#8ECAE6',
    //         },
    //         label: {
    //             text: 'Model',
    //             fontSize: 16,
    //             y: -10
    //         }
    //     },
    //     ports: {
    //         groups: {
    //             'in': portsIn,
    //             'out': portsOut
    //         }
    //     }
    // });

    // model.addPorts([
    //     {
    //         group: 'in',
    //         attrs: { label: { text: 'in1' } }
    //     },
    //     {
    //         group: 'in',
    //         attrs: { label: { text: 'in2' } }
    //     },
    //     {
    //         group: 'out',
    //         attrs: { label: { text: 'out' } }
    //     }
    // ]);

    // model.addTo(graph);
    

    // var rect = new joint.shapes.standard.Rectangle();
    // rect.position(100, 30);
    // rect.resize(100, 40);
    // rect.attr({
    //     body: {
    //         fill: 'blue'
    //     },
    //     label: {
    //         text: 'Hello',
    //         fill: 'white'
    //     }
    // });
    // rect.addTo(graph);

    // var rect2 = new joint.shapes.standard.Rectangle();
    // rect2.position(400, 30);
    // rect2.resize(100, 40);
    // rect2.attr({
    //     body: {
    //         fill: '#2C3E50',
    //         rx: 5,
    //         ry: 5,
    //         strokeWidth: 2
    //     },
    //     label: {
    //         text: 'World!',
    //         fill: '#3498DB',
    //         fontSize: 18,
    //         fontWeight: 'bold',
    //         fontVariant: 'small-caps'
    //     }
    // });
    // rect2.addTo(graph);

    // var link = new joint.shapes.standard.Link();
    // link.source(rect);
    // link.target(rect2);
    // link.attr({
    //     line: {
    //         stroke: 'blue',
    //         strokeWidth: 1,
    //         sourceMarker: {
    //             'type': 'path',
    //             'stroke': 'black',
    //             'fill': 'red',
    //             'd': 'M 10 -5 0 0 10 5 Z'
    //         },
    //         targetMarker: {
    //             'type': 'path',
    //             'stroke': 'black',
    //             'fill': 'yellow',
    //             'd': 'M 10 -5 0 0 10 5 Z'
    //         }
    //     }
    // });
    // link.labels([{
    //     attrs: {
    //         text: {
    //             text: 'Hello, World!'
    //         }
    //     }
    // }]);
    // link.addTo(graph);

    // var rect3 = new joint.shapes.standard.Rectangle();
    // rect3.position(100, 130);
    // rect3.resize(100, 40);
    // rect3.attr({
    //     body: {
    //         fill: '#E74C3C',
    //         rx: 20,
    //         ry: 20,
    //         strokeWidth: 0
    //     },
    //     label: {
    //         text: 'Hello',
    //         fill: '#ECF0F1',
    //         fontSize: 11,
    //         fontVariant: 'small-caps'
    //     }
    // });
    // rect3.addTo(graph);

    // var rect4 = new joint.shapes.standard.Rectangle();
    // rect4.position(400, 130);
    // rect4.resize(100, 40);
    // rect4.attr({
    //     body: {
    //         fill: '#8E44AD',
    //         strokeWidth: 0
    //     },
    //     label: {
    //         text: 'World!',
    //         fill: 'white',
    //         fontSize: 13
    //     }
    // });
    // rect4.addTo(graph);

    // var link2 = new joint.shapes.standard.Link();
    // link2.source(rect3);
    // link2.target(rect4);
    // link2.vertices([
    //     new joint.g.Point(250, 100),
    //     new joint.g.Point(300, 150),
    //     new joint.g.Point(350, 200)
    // ]);
    // link2.router('orthogonal');
    // link2.connector('rounded');
    // link2.attr({
    //     line: {
    //         stroke: 'gray',
    //         strokeWidth: 4,
    //         strokeDasharray: '4 2',
    //         sourceMarker: {
    //             'type': 'image',
    //             'xlink:href': 'https://cdn3.iconfinder.com/data/icons/49handdrawing/24x24/left.png',
    //             'width': 24,
    //             'height': 24,
    //             'y': -12
    //         },
    //         targetMarker: {
    //             'type': 'image',
    //             'xlink:href': 'https://cdn3.iconfinder.com/data/icons/49handdrawing/24x24/left.png',
    //             'width': 24,
    //             'height': 24,
    //             'y': -12
    //         }
    //     }
    // });
    // link2.addTo(graph);

    // var link3 = new joint.shapes.standard.Link();
    // link3.source(rect3);
    // link3.target(rect4);
    // link3.connector('jumpover', { size: 10 });
    // link3.addTo(graph);

    // var rect5 = new joint.shapes.standard.Rectangle();
    // rect5.position(100, 230);
    // rect5.resize(100, 40);
    // rect5.attr({
    //     body: {
    //         fill: '#2ECC71',
    //         strokeDasharray: '10,2'
    //     },
    //     label: {
    //         text: 'Hello',
    //         fill: 'black',
    //         fontSize: 13
    //     }
    // });
    // rect5.addTo(graph);

    // var rect6 = new joint.shapes.standard.Rectangle();
    // rect6.position(400, 230);
    // rect6.resize(100, 40);
    // rect6.attr({
    //     body: {
    //         fill: '#F39C12',
    //         rx: 20,
    //         ry: 20,
    //         strokeDasharray: '1,1'
    //     },
    //     label: {
    //         text: 'World!',
    //         fill: 'gray',
    //         fontSize: 18,
    //         fontWeight: 'bold',
    //         fontVariant: 'small-caps',
    //         textShadow: '1px 1px 1px black'
    //     }
    // });
    // rect6.addTo(graph);

    // var link4 = new joint.shapes.standard.Link();
    // link4.source(rect5);
    // link4.target(rect6);
    // link4.attr({
    //     line: {
    //         stroke: '#3498DB',
    //         strokeWidth: 3,
    //         strokeDasharray: '5 5',
    //         strokeDashoffset: 7.5,
    //         sourceMarker: {
    //             'type': 'path',
    //             'stroke': 'none',
    //             'fill': '#3498DB',
    //             'd': 'M 20 -10 0 0 20 10 Z \
    //                     M 40 -10 20 0 40 10 Z'
    //         },
    //         targetMarker: {
    //             'type': 'path',
    //             'stroke': 'none',
    //             'fill': '#3498DB',
    //             'd': 'M 7.5 -10 2.5 -10 2.5 10 7.5 10 Z \
    //                     M 17.5 -10 12.5 -10 12.5 10 17.5 10 Z \
    //                     M 40 -10 20 0 40 10 Z'
    //         }
    //     }
    // });
    // link4.addTo(graph);

    // var element = new CustomTextElement();


    // element.position(200, 200);
    // element.resize(100, 100);
    // element.attr({
    //     label: {
    //         text: 'Hello,!'
    //     },
    //     e: {
    //         ref: 'label',
    //         rx: 'calc(0.5*w)',
    //         ry: 'calc(0.25*h)',
    //         cx: '-calc(0.5*w)',
    //         cy: '-calc(0.25*h)'
    //     },
    //     r: {
    //         ref: 'label',
    //         // additional x offset
    //         x: 'calc(0.5*w-10)',
    //         // additional y offset
    //         y: 'calc(0.5*h-10)',
    //         width: 'calc(0.5*w)',
    //         height: 'calc(0.5*h)'
    //     },
    //     c: {
    //         ref: 'label',
    //         r: 'calc(0.5*d)'
    //         // c is already centered at label anchor
    //     }
    // });
    // element.addTo(graph);

    // var elementView = element.findView(paper);
    // elementView.addTools(toolsView);

    // paper.on('element:mouseenter', function (elementView) {
    //     elementView.showTools();
    // });

    // paper.on('element:mouseleave', function (elementView) {
    //     elementView.hideTools();
    // });




});
</script>