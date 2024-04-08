import * as joint from 'jointjs';

export class RegELement extends joint.dia.Element {
    defaults() {
        return {
            ...super.defaults,
            type: 'example.RegELement',
            attrs: {
                foreignObject: {
                    width: 'calc(w)',
                    height: 'calc(h)'
                }
            }
        };
    }

    preinitialize() {
        this.markup = joint.util.svg/* xml */`
            <foreignObject @selector="foreignObject">
                <div
                    xmlns="http:www.w3.org/1999/xhtml"
                    style="background:white;border:1px solid black;height:100%;display:flex;justify-content:center;align-items:center;"
                >
                    <input
                        placeholder="Type something"
                    />
                </div>
            </foreignObject>
        `;
    }
}

// const RegELementView = joint.dia.ElementView.extend({

//     events: {
//         // Name of event + CSS selector : custom view method name
//         'input input': 'onInput'
//     },

//     onInput: function(evt) {
//         console.log('Input Value:', evt.target.value);
//         this.model.attr('name/props/value', evt.target.value);
//     }
// });

// Object.assign(namespace, {
//     example: {
//         RegELement,
//         RegELementView
//     }
// });

