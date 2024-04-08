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
            cellViewNamespace: this.pg_namespace,
            defaultLink: () => new joint.shapes.standard.Link({
                attrs: {
                    wrapper: {
                        cursor: 'default'
                    }
                }
            }),
            linkPinning: false,
            validateConnection: function (cellViewS, magnetS, cellViewT, magnetT, end, linkView) {
                // Prevent linking from output ports to input ports within one element
                if (cellViewS === cellViewT) return false;
                // Prevent linking to output ports
                return magnetT && magnetT.getAttribute('port-group') === 'in';
            },
            validateMagnet: function (cellView, magnet) {
                // Note that this is the default behaviour. It is shown for reference purposes.
                // Disable linking interaction for magnets marked as passive
                return magnet.getAttribute('magnet') !== 'passive';
            },
            // Enable link snapping within 20px lookup radius
            snapLinks: { radius: 20 }
        });



        var boundaryTool = new joint.elementTools.Boundary();
        var removeButton = new joint.elementTools.Remove();

        this.pg_toolview = new joint.dia.ToolsView({
            tools: [
                boundaryTool,
                removeButton
            ]
        });

        this.pg_graph.on('add', function (cell) {
            if (cell.isLink()) {
                var source = cell.get('source');
                // if (source.portBody.portid === 'portin') {
                //     // 这里可以确定这个链接是从id为uniquePortId1的端口开始的
                //     console.log('port in !');
                // }
            }
        });


        // Register events
        this.pg_paper.on('link:mouseenter', (linkView) => {
            this.showLinkTools(linkView);
        });

        this.pg_paper.on('link:mouseleave', (linkView) => {
            linkView.removeTools();
        });


        // pg_paper.on('element:mouseenter', function (elementView) {
        //     elementView.showTools();
        // });

        // pg_paper.on('element:mouseleave', function (elementView) {
        //     elementView.hideTools();
        // });


    }

    addElement(eposx, eposy) {
        var portsIn = {
            position: {
                name: 'left'
            },
            attrs: {
                portBody: {
                    magnet: 'passive',
                    r: 5,
                    fill: '#023047',
                    stroke: '#023047',
                    portid: 'portin'
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
            position: { x: eposx, y: eposy },
            size: { width: 90, height: 90 },
            attrs: {
                body: {
                    fill: '#8ECAE6',
                    rx: 10,
                    ry: 10,
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
                attrs: { label: { text: 'in' } }
            },
            {
                group: 'out',
                attrs: { label: { text: 'out' } }
            }
        ]);

        model.addTo(this.pg_graph);

        model.on('mouseenter', function (cellView) {
            cellView.model.attr('ports/items', { attrs: { circle: { visibility: 'visible' } } });
        });

        model.on('mouseleave', function (cellView) {
            cellView.model.attr('ports/items', { attrs: { circle: { visibility: 'hidden' } } });
        });

        // var elementView = model.findView(this.pg_paper);
        // elementView.addTools(this.pg_toolview);
    }

    // 其他方法...
    showLinkTools(linkView) {
        var tools = new joint.dia.ToolsView({
            tools: [
                new joint.linkTools.Remove({
                    distance: '50%',
                    markup: [{
                        tagName: 'circle',
                        selector: 'button',
                        attributes: {
                            'r': 7,
                            'fill': '#f6f6f6',
                            'stroke': '#ff5148',
                            'stroke-width': 2,
                            'cursor': 'pointer'
                        }
                    }, {
                        tagName: 'path',
                        selector: 'icon',
                        attributes: {
                            'd': 'M -3 -3 3 3 M -3 3 3 -3',
                            'fill': 'none',
                            'stroke': '#ff5148',
                            'stroke-width': 2,
                            'pointer-events': 'none'
                        }
                    }]
                })
            ]
        });
        linkView.addTools(tools);
    }
}

