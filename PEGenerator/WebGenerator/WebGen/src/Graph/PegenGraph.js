import * as joint from 'jointjs';

export class PegenGraph {
    static instance;
    pg_namespace = null;
    pg_graph = null;
    pg_paper = null;
    pg_toolview = null;

    constructor() {
        if (PegenGraph.instance) {
            return PegenGraph.instance;
        }
        PegenGraph.instance = this;
        // 初始化操作
        this.data = {}; // 示例属性
    }

    init(pg_div) {

        var CustomTextElement = joint.dia.Element.define('examples.CustomTextElement', {
            attrs: {
                label: {
                    textAnchor: 'middle',
                    textVerticalAnchor: 'middle',
                    fontSize: 48
                },
                e: {
                    strokeWidth: 1,
                    stroke: '#000000',
                    fill: 'rgba(255,0,0,0.3)'
                },
                r: {
                    strokeWidth: 1,
                    stroke: '#000000',
                    fill: 'rgba(0,255,0,0.3)'
                },
                c: {
                    strokeWidth: 1,
                    stroke: '#000000',
                    fill: 'rgba(0,0,255,0.3)'
                },
                outline: {
                    ref: 'label',
                    x: '-calc(0.5*w)',
                    y: '-calc(0.5*h)',
                    width: 'calc(w)',
                    height: 'calc(h)',
                    strokeWidth: 1,
                    stroke: '#000000',
                    strokeDasharray: '5 5',
                    strokeDashoffset: 2.5,
                    fill: 'none'
                }
            }
        }, {
            markup: [{
                tagName: 'ellipse',
                selector: 'e'
            }, {
                tagName: 'rect',
                selector: 'r'
            }, {
                tagName: 'circle',
                selector: 'c'
            }, {
                tagName: 'text',
                selector: 'label'
            }, {
                tagName: 'rect',
                selector: 'outline'
            }]
        });

        this.pg_namespace = joint.shapes;
        this.pg_namespace.CustomTextElement = CustomTextElement;
        this.pg_graph = new joint.dia.Graph({}, { cellNamespace: this.pg_namespace });
        const prect = pg_div.value.getBoundingClientRect();
        this.pg_paper = new joint.dia.Paper({
            el: pg_div.value,
            model: this.pg_graph,
            width: prect.width,
            height: prect.height,
            gridSize: 10,
            drawGrid: true,
            background: {
                color: 'rgba(0, 255, 0, 0.3)'
            },
            cellViewNamespace: this.pg_namespace
        });



        var boundaryTool = new joint.elementTools.Boundary();
        var removeButton = new joint.elementTools.Remove();

        this.pg_toolview = new joint.dia.ToolsView({
            tools: [
                boundaryTool,
                removeButton
            ]
        });

    
        
        // pg_paper.on('element:mouseenter', function (elementView) {
        //     elementView.showTools();
        // });

        // pg_paper.on('element:mouseleave', function (elementView) {
        //     elementView.hideTools();
        // });


    }

    addElement() {
        var portsIn = {
            position: {
                name: 'left'
            },
            attrs: {
                portBody: {
                    magnet: true,
                    r: 5,
                    fill: '#023047',
                    stroke: '#023047'
                }
            },
            label: {
                position: {
                    name: 'left',
                    args: { y: 6 }
                },
                markup: [{
                    tagName: 'text',
                    selector: 'label',
                    className: 'label-text'
                }]
            },
            markup: [{
                tagName: 'circle',
                selector: 'portBody'
            }]
        };

        var portsOut = {
            position: {
                name: 'right'
            },
            attrs: {
                portBody: {
                    magnet: true,
                    r: 5,
                    fill: '#E6A502',
                    stroke: '#023047'
                }
            },
            label: {
                position: {
                    name: 'right',
                    args: { y: 6 }
                },
                markup: [{
                    tagName: 'text',
                    selector: 'label',
                    className: 'label-text'
                }]
            },
            markup: [{
                // Markup
                tagName: 'circle',
                selector: 'portBody'
            }]
        };

        var model = new joint.shapes.standard.Rectangle({
            position: { x: 275, y: 50 },
            size: { width: 90, height: 90 },
            attrs: {
                body: {
                    fill: '#8ECAE6',
                },
                label: {
                    text: 'Model',
                    fontSize: 16,
                    y: -10
                }
            },
            ports: {
                groups: {
                    'in': portsIn,
                    'out': portsOut
                }
            }
        });

        model.addPorts([
            {
                group: 'in',
                attrs: { label: { text: 'in1' } }
            },
            {
                group: 'in',
                attrs: { label: { text: 'in2' } }
            },
            {
                group: 'out',
                attrs: { label: { text: 'out' } }
            }
        ]);

        model.addTo(this.pg_graph);

        // var elementView = model.findView(this.pg_paper);
        // elementView.addTools(this.pg_toolview);
    }

    // 其他方法...
}

