"""Inspect and restore the manifest-listed Content files without changing Git state."""
from pathlib import Path
import argparse,hashlib,json,shutil,sys

ROOT=Path(__file__).resolve().parents[1]
MANIFEST=ROOT/'Docs/AssetManifest.json'

def sha256(file):
    digest=hashlib.sha256()
    with file.open('rb') as stream:
        for block in iter(lambda:stream.read(1024*1024),b''):
            digest.update(block)
    return digest.hexdigest()

def contained(root,relative):
    path=(root/relative).resolve()
    if not path.is_relative_to(root.resolve()) or not relative.startswith('Content/'):
        raise ValueError('Manifest path escaped Content: '+relative)
    return path

def state(root,item):
    path=contained(root,item['path'])
    if not path.is_file():return 'missing'
    if path.stat().st_size<1024:
        with path.open('rb') as stream:
            if stream.read(48).startswith(b'version https://git-lfs.github.com/spec/v1'):
                return 'lfs_pointer'
    if path.stat().st_size!=item['bytes']:return 'different'
    return 'ok' if sha256(path)==item['sha256'] else 'different'

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    sub=parser.add_subparsers(dest='command',required=True)
    check=sub.add_parser('check');check.add_argument('--all',action='store_true');check.add_argument('--root',type=Path,default=ROOT)
    restore=sub.add_parser('restore');restore.add_argument('--source',type=Path,required=True);restore.add_argument('--dry-run',action='store_true')
    args=parser.parse_args();manifest=json.loads(MANIFEST.read_text(encoding='utf-8'))
    if args.command=='check':
        wanted=[x for x in manifest['assets'] if args.all or x['distribution']=='included']
        counts={};bad=[]
        for item in wanted:
            result=state(args.root,item);counts[result]=counts.get(result,0)+1
            if result!='ok':bad.append({'path':item['path'],'state':result})
        print(json.dumps({'scope':'all' if args.all else 'public','checked':len(wanted),'states':counts,'issues':bad[:30],'issueCount':len(bad),'externalRestoreFiles':manifest['summary']['restore']['files']},ensure_ascii=False,indent=2))
        return 1 if bad else 0
    source=args.source.resolve();wanted=[x for x in manifest['assets'] if x['distribution']=='restore'];prepared=[];issues=[]
    for item in wanted:
        src=contained(source,item['path']);dest=contained(ROOT,item['path']);result=state(source,item)
        if result!='ok':issues.append({'path':item['path'],'sourceState':result});continue
        if dest.exists():
            if state(ROOT,item)!='ok':issues.append({'path':item['path'],'destinationState':'different; overwrite refused'})
            continue
        prepared.append((src,dest))
    if issues:
        print(json.dumps({'copied':0,'issueCount':len(issues),'issues':issues[:30]},ensure_ascii=False,indent=2));return 1
    if not args.dry_run:
        for src,dest in prepared:
            dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(src,dest)
    print(json.dumps({'dryRun':args.dry_run,'validated':len(wanted),'toCopy':len(prepared),'copied':0 if args.dry_run else len(prepared)},indent=2));return 0

if __name__=='__main__':
    sys.exit(main())
